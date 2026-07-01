// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to replace text in all pages in PDF document.

// Include Foxit SDK header files.
#include <iostream>
#include <string>

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/addon/pageeditor/fs_searchreplace.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace foxit::addon::pageeditor;

static const char* sn = "";
static const char* key = "";

struct SearchReplaceCommand {
  WString input_file;
  WString output_file;
  WString pattern;
  WString replacement;
  bool match_case;
  bool match_whole_word;
  int max_replace;
  bool show_help;

  SearchReplaceCommand()
      : match_case(false),
        match_whole_word(false),
        max_replace(10),
        show_help(false) {}
};

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

void PrintUsage() {
  cout << "Usage:" << endl;
  cout << "searchreplace --input <input.pdf> --output <output.pdf> --pattern <search text> --replacement <replace text> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --input <path>                  Input PDF path." << endl;
  cout << "  --output <path>                 Output PDF path." << endl;
  cout << "  --pattern <text>                Search pattern text." << endl;
  cout << "  --replacement <text>            Replacement text." << endl << endl;
  cout << "Optional:" << endl;
  cout << "  --match-case                    Match case (default: false)." << endl;
  cout << "  --match-whole-word              Match whole word (default: false)." << endl;
  cout << "  --max-replace <count>           Max replacements (default: 10, 0 = unlimited)." << endl;
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

bool ParseCommand(int argc, char* argv[], SearchReplaceCommand& command) {
  if (argc <= 1) {
    PrintUsage();
    return false;
  }

  bool has_input = false;
  bool has_output = false;
  bool has_pattern = false;
  bool has_replacement = false;
  for (int i = 1; i < argc; ++i) {
    String key = argv[i];
    if (key.Equal("--help")) {
      command.show_help = true;
      PrintUsage();
      return false;
    }
    if (key.Equal("--match-case")) {
      command.match_case = true;
      continue;
    }
    if (key.Equal("--match-whole-word")) {
      command.match_whole_word = true;
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
    } else if (key.Equal("--pattern")) {
      command.pattern = WString::FromUTF8(value);
      has_pattern = true;
    } else if (key.Equal("--replacement")) {
      command.replacement = WString::FromUTF8(value);
      has_replacement = true;
    } else if (key.Equal("--max-replace")) {
      command.max_replace = atoi(value);
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (!has_input || !has_output || !has_pattern || !has_replacement) {
    printf("--input, --output, --pattern, and --replacement are required.\n");
    PrintUsage();
    return false;
  }
  if (!FileExists(command.input_file)) {
    printf("Input file does not exist: %s\n", (const char*)String::FromUnicode(command.input_file));
    return false;
  }
  return true;
}

class ReplaceCallbackImpl : public ReplaceCallback {
public:
  explicit ReplaceCallbackImpl(int max_replace) : count(0), max_replace_(max_replace) {}
  virtual bool NeedToReplace(const WString& search_text, const WString& replace_text, int current_page_index, const RectFArray& text_rect_array) {
    count++;
    if (max_replace_ == 0 || count <= max_replace_) return true;
    else return false;
  }

  virtual void Release() {
    delete this;
  }

private:
  int count;
  int max_replace_;
};

int main(int argc, char* argv[]) {
  SearchReplaceCommand command;
  if (!ParseCommand(argc, argv, command)) return 1;

  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  int err_ret = 0;
  try {
    PDFDoc doc(command.input_file);
    ErrorCode error_code = doc.Load();
    if (error_code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(command.input_file), error_code);
      return 1;
    }
    TextSearchReplace text_searchreplace(doc);
    FindOption find_option(command.match_case, command.match_whole_word);
    ReplaceCallbackImpl* replace_callback = new ReplaceCallbackImpl(command.max_replace);
    text_searchreplace.SetReplaceCallback(replace_callback);
    text_searchreplace.SetPattern(command.pattern, 0, find_option);
    while (text_searchreplace.ReplaceNext(command.replacement)) {}

    doc.SaveAs(command.output_file);
    wcout << L"Search Replace demo finished." << endl;

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

