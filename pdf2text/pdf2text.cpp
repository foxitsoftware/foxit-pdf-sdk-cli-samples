// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to extract text from PDF document.

// Include Foxit SDK header files.
#include <iostream>
#include <string>

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/fs_search.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "";
static const char* key = "";

struct Pdf2TextCommand {
  WString input_file;
  WString output_file;
  bool show_help;

  Pdf2TextCommand()
      : show_help(false) {}
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
  cout << "pdf2text --input <input.pdf> --output <output.txt> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --input <path>                  Input PDF path." << endl;
  cout << "  --output <path>                 Output text file path." << endl << endl;
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

bool ParseCommand(int argc, char* argv[], Pdf2TextCommand& command) {
  if (argc <= 1) {
    PrintUsage();
    return false;
  }

  bool has_input = false;
  bool has_output = false;
  for (int i = 1; i < argc; ++i) {
    String key = argv[i];
    if (key.Equal("--help")) {
      command.show_help = true;
      PrintUsage();
      return false;
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
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (!has_input || !has_output) {
    printf("Both --input and --output are required.\n");
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
  Pdf2TextCommand command;
  if (!ParseCommand(argc, argv, command)) return 1;

  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) return 1;

  // Load a PDF document.
  PDFDoc doc(command.input_file);
  error_code = doc.Load();
  if (error_code != foxit::e_ErrSuccess) {
    printf("Error: Load PDF document \"%s\" failed. Error code: %d\n",
           (FX_LPCSTR)String::FromUnicode(command.input_file), error_code);
    return 1;
  }

  // Extract text from each page.
  int page_count = doc.GetPageCount();
  String text_content;
  for (int i = 0; i < page_count; i++) {
    PDFPage page = doc.GetPage(i);
    if (page.IsEmpty()) {
      printf("Warning: Page %d is empty, skipped.\n", i);
      continue;
    }
    page.StartParse();
    TextPage text_page(page);
    int char_count = text_page.GetCharCount();
    if (char_count > 0) {
      WString wtext = text_page.GetChars();
      String text = wtext.UTF8Encode();
      text_content += text;
      if (i < page_count - 1) {
        text_content += "\n\n";
      }
    }
  }

  // Save extracted text to the output file.
#if defined(_WIN32) || defined(_WIN64)
  FILE* text_file = _wfopen((const wchar_t*)command.output_file, L"wt");
#else
  FILE* text_file = fopen((const char*)String::FromUnicode(command.output_file), "wt");
#endif
  if (text_file) {
    fprintf(text_file, "%s", (FX_LPCSTR)text_content);
    fclose(text_file);
    printf("Extracted text has been saved to \"%s\".\n",
           (FX_LPCSTR)String::FromUnicode(command.output_file));
  } else {
    printf("Error: Failed to save text to \"%s\".\n",
           (FX_LPCSTR)String::FromUnicode(command.output_file));
    return 1;
  }

  return 0;
}

