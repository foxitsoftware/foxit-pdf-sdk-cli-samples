// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to add, delete and modify
// the header-footer of PDF file.

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
#include "../../../include/pdf/fs_headerfooter.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

struct PdfHeaderFooterCommand {
  WString input_file;
  WString output_file;
  WString action;           // add, update, remove
  WString header_left;
  WString header_center;
  WString header_right;
  WString footer_left;
  WString footer_center;
  WString footer_right;
  float text_size;
  RGB text_color;
  WString font_name;
  WString font_file;
  int font_face_index;
  int start_page;           // page range start (1-based)
  int end_page;             // page range end (1-based)
  WString page_filter;      // all, odd, even
  float margin_left;
  float margin_bottom;
  float margin_right;
  float margin_top;
  int start_page_number;    // virtual page number for first page
  bool embed_font;
  bool underline;
  bool shrink_text;
  bool fixed_size_print;
  bool show_help;

  PdfHeaderFooterCommand()
      : text_size(8),
        text_color(0x000000),
        start_page(1),
        end_page(0),
        margin_left(72.0f),
        margin_bottom(36.0f),
        margin_right(72.0f),
        margin_top(36.0f),
        start_page_number(1),
        font_face_index(0),
        embed_font(false),
        underline(false),
        shrink_text(false),
        fixed_size_print(false),
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
  cout << "pdfheaderfooter --action <add|update|remove> --input <input.pdf> --output <output.pdf> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --action <type>                 Action: add | update | remove." << endl;
  cout << "  --input <path>                  Input PDF file path." << endl;
  cout << "  --output <path>                 Output PDF file path." << endl << endl;
  cout << "Content (for add/update):" << endl;
  cout << "  --header-left <text>            Left header content." << endl;
  cout << "  --header-center <text>          Center header content." << endl;
  cout << "  --header-right <text>           Right header content." << endl;
  cout << "  --footer-left <text>            Left footer content." << endl;
  cout << "  --footer-center <text>          Center footer content." << endl;
  cout << "  --footer-right <text>           Right footer content." << endl << endl;
  cout << "Style (for add/update):" << endl;
  cout << "  --text-size <float>             Text size. Default: 8." << endl;
  cout << "  --text-color <hex>              Text color (e.g. 0xFF0000). Default: 0x000000." << endl;
  cout << "  --font-name <name>              Font name (e.g. Arial). Mutually exclusive with --font-file." << endl;
  cout << "  --font-file <path>              Font file path (.ttf/.otf etc). Mutually exclusive with --font-name." << endl;
  cout << "  --font-face-index <int>         Face index within font file. Default: 0." << endl;
  cout << "  --embed-font                    Embed font. Default: off." << endl;
  cout << "  --underline                     Underline text. Default: off." << endl << endl;
  cout << "Page range (for add/update):" << endl;
  cout << "  --start-page <int>              Start page (1-based). Default: 1." << endl;
  cout << "  --end-page <int>                End page (1-based). Default: last page." << endl;
  cout << "  --page-filter <type>            Page filter: all | odd | even. Default: all." << endl;
  cout << "  --start-page-number <int>       Virtual page number for first page. Default: 1." << endl << endl;
  cout << "Margin (for add/update):" << endl;
  cout << "  --margin-left <float>           Left margin. Default: 72." << endl;
  cout << "  --margin-bottom <float>         Bottom margin. Default: 36." << endl;
  cout << "  --margin-right <float>          Right margin. Default: 72." << endl;
  cout << "  --margin-top <float>            Top margin. Default: 36." << endl << endl;
  cout << "Other (for add/update):" << endl;
  cout << "  --shrink-text                   Shrink text to fit. Default: off." << endl;
  cout << "  --fixed-size-print              Use fixed size for print. Default: off." << endl << endl;
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

bool ParseCommand(int argc, char* argv[], PdfHeaderFooterCommand& command) {
  if (argc <= 1) {
    PrintUsage();
    return false;
  }

  bool has_input = false;
  bool has_output = false;
  bool has_action = false;
  for (int i = 1; i < argc; ++i) {
    String key = argv[i];
    if (key.Equal("--help")) {
      command.show_help = true;
      PrintUsage();
      return false;
    }
    // Flag options (no value)
    if (key.Equal("--embed-font")) {
      command.embed_font = true;
      continue;
    }
    if (key.Equal("--underline")) {
      command.underline = true;
      continue;
    }
    if (key.Equal("--shrink-text")) {
      command.shrink_text = true;
      continue;
    }
    if (key.Equal("--fixed-size-print")) {
      command.fixed_size_print = true;
      continue;
    }
    if (i + 1 >= argc) {
      printf("Missing value for option: %s\n", (const char*)key);
      return false;
    }

    String value = argv[++i];
    if (key.Equal("--action") || key.Equal("-a")) {
      if (!value.Equal("add") && !value.Equal("update") && !value.Equal("remove")) {
        printf("Invalid action: %s (must be add, update, or remove)\n", (const char*)value);
        return false;
      }
      command.action = WString::FromUTF8(value);
      has_action = true;
    } else if (key.Equal("--input") || key.Equal("-i")) {
      command.input_file = WString::FromUTF8(value);
      has_input = true;
    } else if (key.Equal("--output") || key.Equal("-o")) {
      command.output_file = WString::FromUTF8(value);
      has_output = true;
    } else if (key.Equal("--header-left")) {
      command.header_left = WString::FromUTF8(value);
    } else if (key.Equal("--header-center")) {
      command.header_center = WString::FromUTF8(value);
    } else if (key.Equal("--header-right")) {
      command.header_right = WString::FromUTF8(value);
    } else if (key.Equal("--footer-left")) {
      command.footer_left = WString::FromUTF8(value);
    } else if (key.Equal("--footer-center")) {
      command.footer_center = WString::FromUTF8(value);
    } else if (key.Equal("--footer-right")) {
      command.footer_right = WString::FromUTF8(value);
    } else if (key.Equal("--text-size")) {
      command.text_size = (float)atof((const char*)value);
    } else if (key.Equal("--text-color")) {
      command.text_color = ParseHexColor(value);
    } else if (key.Equal("--font-name")) {
      command.font_name = WString::FromUTF8(value);
    } else if (key.Equal("--font-file")) {
      command.font_file = WString::FromUTF8(value);
    } else if (key.Equal("--font-face-index")) {
      command.font_face_index = atoi((const char*)value);
    } else if (key.Equal("--start-page")) {
      command.start_page = atoi((const char*)value);
    } else if (key.Equal("--end-page")) {
      command.end_page = atoi((const char*)value);
    } else if (key.Equal("--page-filter")) {
      if (!value.Equal("all") && !value.Equal("odd") && !value.Equal("even")) {
        printf("Invalid page filter: %s (must be all, odd, or even)\n", (const char*)value);
        return false;
      }
      command.page_filter = WString::FromUTF8(value);
    } else if (key.Equal("--start-page-number")) {
      command.start_page_number = atoi((const char*)value);
    } else if (key.Equal("--margin-left")) {
      command.margin_left = (float)atof((const char*)value);
    } else if (key.Equal("--margin-bottom")) {
      command.margin_bottom = (float)atof((const char*)value);
    } else if (key.Equal("--margin-right")) {
      command.margin_right = (float)atof((const char*)value);
    } else if (key.Equal("--margin-top")) {
      command.margin_top = (float)atof((const char*)value);
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (!has_action || !has_input || !has_output) {
    printf("--action, --input, and --output are all required.\n");
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
  PdfHeaderFooterCommand command;
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

    String action = String::FromUnicode(command.action);

    if (action.Equal("remove")) {
      doc.RemoveAllHeaderFooters();
      doc.SaveAs(command.output_file, PDFDoc::e_SaveFlagNoOriginal);
      cout << "All header-footers removed. Output: " << (const char*)String::FromUnicode(command.output_file) << endl;

    } else {
      // add or update
      HeaderFooter headerfooter;
      if (action.Equal("update")) {
        headerfooter = doc.GetEditableHeaderFooter();
      }

      // Set content
      HeaderFooterContent content = headerfooter.content;
      if (!command.header_left.IsEmpty()) content.header_left_content = command.header_left;
      if (!command.header_center.IsEmpty()) content.header_center_content = command.header_center;
      if (!command.header_right.IsEmpty()) content.header_right_content = command.header_right;
      if (!command.footer_left.IsEmpty()) content.footer_left_content = command.footer_left;
      if (!command.footer_center.IsEmpty()) content.footer_center_content = command.footer_center;
      if (!command.footer_right.IsEmpty()) content.footer_right_content = command.footer_right;
      headerfooter.content = content;

      // Set style
      headerfooter.text_size = command.text_size;
      headerfooter.text_color = command.text_color;
      if (!command.font_file.IsEmpty()) {
        headerfooter.font = common::Font((const wchar_t*)command.font_file, command.font_face_index, common::Font::e_CharsetANSI);
      } else if (!command.font_name.IsEmpty()) {
        headerfooter.font = common::Font((const wchar_t*)command.font_name, common::Font::e_StyleFixedPitch, common::Font::e_CharsetANSI, 0);
      }
      headerfooter.is_to_embed_font = command.embed_font;
      headerfooter.is_underline = command.underline;

      // Set page range
      int end_page = (command.end_page > 0) ? command.end_page : doc.GetPageCount();
      foxit::common::Range::Filter filter = foxit::common::Range::e_All;
      String page_filter = String::FromUnicode(command.page_filter);
      if (page_filter.Equal("odd")) filter = foxit::common::Range::e_Odd;
      else if (page_filter.Equal("even")) filter = foxit::common::Range::e_Even;
      headerfooter.page_range = PageNumberRange(command.start_page, end_page, filter);

      // Set margin
      headerfooter.page_margin = RectF(command.margin_left, command.margin_bottom,
                                        command.margin_right, command.margin_top);

      // Set other properties
      headerfooter.start_page_number = command.start_page_number;
      headerfooter.has_text_shrinked = command.shrink_text;
      headerfooter.has_fixedsize_for_print = command.fixed_size_print;

      if (action.Equal("add")) {
        doc.AddHeaderFooter(headerfooter);
        doc.SaveAs(command.output_file, PDFDoc::e_SaveFlagNoOriginal);
        cout << "Header-footer added. Output: " << (const char*)String::FromUnicode(command.output_file) << endl;
      } else {
        doc.UpdateHeaderFooter(headerfooter);
        doc.SaveAs(command.output_file, PDFDoc::e_SaveFlagNoOriginal);
        cout << "Header-footer updated. Output: " << (const char*)String::FromUnicode(command.output_file) << endl;
      }
    }
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

