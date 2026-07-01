// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to redact some content in a PDF document.

#include <iostream>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/fs_search.h"
#include "../../../include/addon/fs_redaction.h"
#include "../../../include/common/fs_render.h"

using namespace std;
using namespace foxit;
using namespace common;
using namespace addon;
using foxit::common::Library;
using namespace pdf;
using namespace foxit::pdf::annots;

static const char* sn = "";
static const char* key = "";

struct PdfRedactCommand {
  WString input_file;
  WString output_file;
  WString text;            // text pattern to search and redact
  int page_index;          // -1 means all pages
  RGB border_color;        // default: red (0xFF0000)
  RGB fill_color;          // default: black (0x000000)
  RGB apply_color;         // default: black (0x000000)
  float opacity;           // default: 1.0
  bool apply;              // whether to apply redaction (default: mark only)
  bool show_help;

  PdfRedactCommand()
      : page_index(-1),
        border_color(0xFF0000),
        fill_color(0x000000),
        apply_color(0x000000),
        opacity(1.0f),
        apply(false),
        show_help(false) {}
};

class SdkLibMgr {
 public:
  SdkLibMgr()
      : is_initialize_(false){};
  ErrorCode Initialize() {
    ErrorCode error_code = Library::Initialize(sn, key);
    if (error_code != foxit::e_ErrSuccess) {
      printf("Library Initialize Error: %d\n", error_code);
    } else {
      is_initialize_ = true;
    }
    return error_code;
  }
  ~SdkLibMgr() {
    if (is_initialize_)
      Library::Release();
  }

 private:
  bool is_initialize_;
};

void PrintUsage() {
  cout << "Usage:" << endl;
  cout << "pdfredact --input <input.pdf> --output <output.pdf> --text <pattern> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --input <path>                  Input PDF file path." << endl;
  cout << "  --output <path>                 Output PDF file path." << endl;
  cout << "  --text <pattern>                Text pattern to search and redact." << endl << endl;
  cout << "Page selection:" << endl;
  cout << "  --page <index>                  Page index to redact (0-based). Default: all pages." << endl << endl;
  cout << "Appearance:" << endl;
  cout << "  --border-color <hex>            Border color (e.g. 0xFF0000). Default: 0xFF0000." << endl;
  cout << "  --fill-color <hex>              Fill color (e.g. 0x000000). Default: 0x000000." << endl;
  cout << "  --apply-color <hex>             Apply fill color after redaction. Default: 0x000000." << endl;
  cout << "  --opacity <float>               Opacity (0.0-1.0). Default: 1.0." << endl << endl;
  cout << "Action:" << endl;
  cout << "  --apply                         Apply redaction (permanently remove content). Default: mark only." << endl << endl;
  cout << "Optional:" << endl;
  cout << "  --help                          Show this message." << endl;
}

bool FileExists(const WString& path) {
  if (path.IsEmpty()) return false;
  FILE* file = NULL;
#if defined(_WIN32) || defined(_WIN64)
  _wfopen_s(&file, (const wchar_t*)path, L"rb");
#else
  file = fopen(String::FromUnicode(path), "rb");
#endif
  if (!file) return false;
  fclose(file);
  return true;
}

RGB ParseHexColor(const String& value) {
  if (value.GetLength() > 2 && (value.GetAt(0) == '0' && (value.GetAt(1) == 'x' || value.GetAt(1) == 'X'))) {
    return (RGB)strtol((const char*)value, NULL, 0);
  }
  return (RGB)atoi((const char*)value);
}

bool ParseCommand(int argc, char* argv[], PdfRedactCommand& command) {
  if (argc <= 1) {
    PrintUsage();
    return false;
  }

  bool has_input = false;
  bool has_output = false;
  bool has_text = false;
  for (int i = 1; i < argc; ++i) {
    String key = argv[i];
    if (key.Equal("--help")) {
      command.show_help = true;
      PrintUsage();
      return false;
    }
    if (key.Equal("--apply")) {
      command.apply = true;
      continue;
    }
    if (i + 1 >= argc) {
      printf("Missing value for option: %s\n", (const char*)key);
      return false;
    }

    String value = argv[++i];
    if (key.Equal("--input") || key.Equal("-i")) {
      command.input_file = WString::FromUTF8(value);
      has_input = true;
    } else if (key.Equal("--output") || key.Equal("-o")) {
      command.output_file = WString::FromUTF8(value);
      has_output = true;
    } else if (key.Equal("--text") || key.Equal("-t")) {
      command.text = WString::FromUTF8(value);
      has_text = true;
    } else if (key.Equal("--page") || key.Equal("-p")) {
      command.page_index = atoi((const char*)value);
    } else if (key.Equal("--border-color")) {
      command.border_color = ParseHexColor(value);
    } else if (key.Equal("--fill-color")) {
      command.fill_color = ParseHexColor(value);
    } else if (key.Equal("--apply-color")) {
      command.apply_color = ParseHexColor(value);
    } else if (key.Equal("--opacity")) {
      command.opacity = (float)atof((const char*)value);
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (!has_input || !has_output || !has_text) {
    printf("--input, --output, and --text are all required.\n");
    PrintUsage();
    return false;
  }
  if (!FileExists(command.input_file)) {
    printf("Input file does not exist: %s\n", (const char*)String::FromUnicode(command.input_file));
    return false;
  }
  return true;
}

int main(int argc, char* argv[]) {
  PdfRedactCommand command;
  if (!ParseCommand(argc, argv, command)) return 1;

  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) return 1;

  try {
    PDFDoc doc(command.input_file);
    error_code = doc.Load();
    if (error_code != foxit::e_ErrSuccess) {
      printf("Error: Load PDF \"%s\" failed. Error code: %d\n",
             (const char*)String::FromUnicode(command.input_file), error_code);
      return 1;
    }

    Redaction redaction(doc);
    int page_count = doc.GetPageCount();
    int start_page = (command.page_index >= 0) ? command.page_index : 0;
    int end_page = (command.page_index >= 0) ? command.page_index + 1 : page_count;

    if (command.page_index >= 0 && command.page_index >= page_count) {
      printf("Error: Page index %d out of range (0-%d).\n", command.page_index, page_count - 1);
      return 1;
    }

    int total_matches = 0;
    for (int i = start_page; i < end_page; i++) {
      PDFPage page = doc.GetPage(i);
      page.StartParse(foxit::pdf::PDFPage::e_ParsePageNormal, NULL, false);
      TextPage text_page(page);
      TextSearch text_search(text_page);
      text_search.SetPattern(command.text);

      RectFArray rect_array;
      while (text_search.FindNext()) {
        rect_array.Append(text_search.GetMatchRects());
      }

      if (rect_array.GetSize() > 0) {
        total_matches += rect_array.GetSize();
        Redact redact = redaction.MarkRedactAnnot(page, rect_array);
        redact.SetBorderColor((long)command.border_color);
        redact.SetFillColor((long)command.fill_color);
        redact.SetApplyFillColor((long)command.apply_color);
        redact.SetOpacity(command.opacity);
        redact.ResetAppearanceStream();
      }
    }

    if (total_matches == 0) {
      printf("No matches found for pattern \"%s\".\n", (const char*)String::FromUnicode(command.text));
      return 1;
    }

    if (command.apply) {
      if (redaction.Apply()) {
        cout << "Redaction applied. " << total_matches << " areas redacted." << endl;
      } else {
        cout << "Redaction apply failed." << endl;
        return 1;
      }
    } else {
      cout << total_matches << " areas marked for redaction (not applied). Use --apply to permanently remove content." << endl;
    }

    doc.SaveAs(command.output_file, PDFDoc::e_SaveFlagNormal);
    cout << "Output: " << (const char*)String::FromUnicode(command.output_file) << endl;
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    return 1;
  }
  catch(...) {
    cout << "Unknown Exception" << endl;
    return 1;
  }

  return 0;
}

