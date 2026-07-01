// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to recognize form in a PDF document.


// Include Foxit SDK header files.
#include <iostream>

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/interform/fs_pdfform.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace annots;
using namespace interform;

struct CliOptions {
  WString input_file;
  WString output_file;
  bool show_help;

  CliOptions() : show_help(false) {}
};

void PrintUsage() {
  cout << "Usage:" << endl
       << "demo_form_recognition -i <input pdf path> -o <output pdf path>" << endl
       << endl
       << "Options:" << endl
       << "  -i, --input              Input PDF path" << endl
       << "  -o, --output             Output PDF path" << endl
       << "  -h, --help               Show this help message" << endl;
}

bool FileExists(const WString& path) {
  FILE* file = NULL;
#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file, String::FromUnicode(path), "rb");
#else
  file = fopen(String::FromUnicode(path), "rb");
#endif
  if (file == NULL) return false;
  fclose(file);
  return true;
}

bool ParseArgs(int argc, char* argv[], CliOptions& options, String& error_message) {
  for (int i = 1; i < argc; ++i) {
    String key = String(argv[i]);
    if (key.Equal("-h") || key.Equal("--help")) {
      options.show_help = true;
      continue;
    }
    if (!(key.Equal("-i") || key.Equal("--input") || key.Equal("-o") || key.Equal("--output"))) {
      error_message = "Unknown parameter: " + key;
      return false;
    }
    if (i + 1 >= argc) {
      error_message = "Missing value for parameter: " + key;
      return false;
    }

    String value = String(argv[++i]);
    if (key.Equal("-i") || key.Equal("--input")) options.input_file = WString::FromUTF8(value);
    else options.output_file = WString::FromUTF8(value);
  }
  return true;
}

bool ValidateArgs(const CliOptions& options, String& error_message) {
  if (options.show_help) return true;

  if (options.input_file.IsEmpty()) {
    error_message = "Missing required parameter: -i/--input";
    return false;
  }
  if (options.output_file.IsEmpty()) {
    error_message = "Missing required parameter: -o/--output";
    return false;
  }
  if (!FileExists(options.input_file)) {
    error_message = "Input file does not exist: " + String::FromUnicode(options.input_file);
    return false;
  }
  if (FileExists(options.output_file)) {
    error_message = "Output file already exists (no-overwrite): " + String::FromUnicode(options.output_file);
    return false;
  }
  return true;
}

static const char* sn = "";
static const char* key = "";

class SdkLibMgr {
public:
  SdkLibMgr() : is_initialize_(false){};
  ErrorCode Initialize() {
    ErrorCode error_code = Library::Initialize(sn, key);
    if (error_code != foxit::e_ErrSuccess) {
      printf("Library Initialize Error: %d\n", error_code);
    } else {
      is_initialize_ = true;
    }
    return error_code;

  }
  ~SdkLibMgr(){
    if(is_initialize_)
      Library::Release();
  }
private:
  bool is_initialize_;
};

int main(int argc, char *argv[])
{
  CliOptions options;
  String error_message;
  if (!ParseArgs(argc, argv, options, error_message)) {
    cout << error_message << endl;
    PrintUsage();
    return 1;
  }
  if (options.show_help) {
    PrintUsage();
    return 0;
  }
  if (!ValidateArgs(options, error_message)) {
    cout << error_message << endl;
    PrintUsage();
    return 1;
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    PDFDoc doc(options.input_file);
    doc.Load();
    interform::Form form(doc);
    int count = form.GetFieldCount();
    cout << "Field count:" << count<< endl;
    cout << "===Start Recognize Form===" << endl;
    //Start to recognize form in current PDF document.
    doc.StartRecognizeForm();
    cout << "===End===" << endl;
    count = form.GetFieldCount();
    cout << "Field count:" << count << endl;
    for (int i = 0; i < count; i++) {
      Field field = form.GetField(i);
      WString str;
      str.Format(L"NO.%d",i+1);
      field.SetValue(str);
    }
    doc.SaveAs(options.output_file, PDFDoc::e_SaveFlagNoOriginal);
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  }
  catch(...)
  {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}
