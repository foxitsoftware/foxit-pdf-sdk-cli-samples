// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to Convert a PDF file to a XML format file.

// Include Foxit SDK header files.
#include <iostream>
#include <string>
#include <cctype>

#include "../../../include/common/fs_common.h"
#include "../../../include/addon/conversion/fs_convert.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "";
static const char* key = "";

struct Pdf2XmlCommand {
  WString input_file;
  WString output_file;
  WString src_file_password;
  WString saved_image_folder_path;
  bool is_force_to_tagged_pdf;
  bool show_help;

  Pdf2XmlCommand()
      : is_force_to_tagged_pdf(true),
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
  cout << "pdf2xml --input <input.pdf> --output <output.xml> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --input <path>                     Input PDF path." << endl;
  cout << "  --output <path>                    Output XML file path." << endl << endl;
  cout << "Optional:" << endl;
  cout << "  --password <text>                  Source PDF file password." << endl;
  cout << "  --image-folder <path>              Folder path to save extracted images." << endl;
  cout << "  --force-tagged <true|false>        Force to use tagged PDF structure (default: true)." << endl;
  cout << "  --help                             Show this message." << endl;
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

bool ParseCommand(int argc, char* argv[], Pdf2XmlCommand& command) {
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
    } else if (key.Equal("--password")) {
      command.src_file_password = WString::FromUTF8(value);
    } else if (key.Equal("--image-folder")) {
      command.saved_image_folder_path = WString::FromUTF8(value);
    } else if (key.Equal("--force-tagged")) {
      string text = (const char*)value;
      for (size_t j = 0; j < text.size(); ++j) text[j] = (char)tolower((unsigned char)text[j]);
      if (text == "true" || text == "1") command.is_force_to_tagged_pdf = true;
      else if (text == "false" || text == "0") command.is_force_to_tagged_pdf = false;
      else { printf("Invalid value for --force-tagged: %s\n", (const char*)value); return false; }
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
  Pdf2XmlCommand command;
  if (!ParseCommand(argc, argv, command)) return 1;

  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) return 1;

  try {
    const wchar_t* password = command.src_file_password.IsEmpty() ? NULL : (const wchar_t*)command.src_file_password;
    const wchar_t* image_folder = command.saved_image_folder_path.IsEmpty() ? NULL : (const wchar_t*)command.saved_image_folder_path;
    bool ret = addon::conversion::Convert::ToXML(command.input_file, password, command.output_file,
                                      image_folder, command.is_force_to_tagged_pdf);
    cout << "Convert PDF file to XML format file " << (ret ? "successfully." : "failed.") << endl;
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    return 1;
  }
  catch(...)
  {
    cout << "Unknown Exception" << endl;
    return 1;
  }

  return 0;
}

