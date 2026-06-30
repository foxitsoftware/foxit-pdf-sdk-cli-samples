// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to do full text search in PDF documents under a folder.

// Include Foxit SDK header files.
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/fts/fs_fulltextsearch.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace foxit::fts;

struct CliOptions {
  WString input_directory;
  WString output_directory;
  String keyword;
  WString db_file_name;
  WString result_file_name;
  int pause_limit;
  bool show_help;

  CliOptions()
      : keyword("Foxit"),
        db_file_name(L"search.db"),
        result_file_name(L"result.txt"),
        pause_limit(5),
        show_help(false) {}
};

void PrintUsage() {
  cout << "Usage:" << endl
       << "demo_fulltextsearch -i <input folder> -o <output folder> [options]" << endl
       << endl
       << "Required:" << endl
       << "  -i, --input-dir          Input folder path for PDF documents" << endl
       << "  -o, --output-dir         Output folder path for search artifacts" << endl
       << endl
       << "Optional:" << endl
       << "  --keyword <text>         Search keyword (default: Foxit)" << endl
       << "  --db-file <name>         Database file name under output folder (default: search.db)" << endl
       << "  --result-file <name>     Result text file name under output folder (default: result.txt)" << endl
       << "  --pause-limit <int>      Pause callback limit (default: 5)" << endl
       << "  -h, --help               Show this help message" << endl
      << endl;
}

bool ParseIntValue(const String& value, int& result) {
  std::string text = std::string((const char*)value);
  char* end_ptr = NULL;
  long parsed = strtol(text.c_str(), &end_ptr, 10);
  if (end_ptr == text.c_str() || *end_ptr != '\0') return false;
  result = (int)parsed;
  return true;
}

bool PathExists(const WString& path) {
#if defined(_WIN32) || defined(_WIN64)
  struct _stat info;
  return _stat(String::FromUnicode(path), &info) == 0;
#else
  struct stat info;
  return stat(String::FromUnicode(path), &info) == 0;
#endif
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

WString EnsureTrailingSlash(const WString& directory) {
  if (directory.IsEmpty()) return directory;
  size_t length = directory.GetLength();
  wchar_t last_char = directory[length - 1];
  if (last_char == L'/' || last_char == L'\\') return directory;
  return directory + L"/";
}

bool ParseArgs(int argc, char* argv[], CliOptions& options, String& error_message) {
  for (int i = 1; i < argc; ++i) {
    String key = String(argv[i]);
    if (key.Equal("-h") || key.Equal("--help")) {
      options.show_help = true;
      continue;
    }

    if (i + 1 >= argc) {
      error_message = "Missing value for parameter: " + key;
      return false;
    }

    String value = String(argv[++i]);
    if (key.Equal("-i") || key.Equal("--input-dir")) options.input_directory = WString::FromUTF8(value);
    else if (key.Equal("-o") || key.Equal("--output-dir")) options.output_directory = WString::FromUTF8(value);
    else if (key.Equal("--keyword")) options.keyword = value;
    else if (key.Equal("--db-file")) options.db_file_name = WString::FromUTF8(value);
    else if (key.Equal("--result-file")) options.result_file_name = WString::FromUTF8(value);
    else if (key.Equal("--pause-limit")) {
      if (!ParseIntValue(value, options.pause_limit) || options.pause_limit < 0) {
        error_message = "Invalid value for --pause-limit: " + value;
        return false;
      }
    } else {
      error_message = "Unknown parameter: " + key;
      return false;
    }
  }
  return true;
}

bool ValidateArgs(CliOptions& options, String& error_message) {
  if (options.show_help) return true;

  if (options.input_directory.IsEmpty()) {
    error_message = "Missing required parameter: -i/--input-dir";
    return false;
  }
  if (options.output_directory.IsEmpty()) {
    error_message = "Missing required parameter: -o/--output-dir";
    return false;
  }
  if (!PathExists(options.input_directory)) {
    error_message = "Input directory does not exist: " + String::FromUnicode(options.input_directory);
    return false;
  }
  if (options.keyword.IsEmpty()) {
    error_message = "Search keyword cannot be empty.";
    return false;
  }
  if (options.db_file_name.IsEmpty()) {
    error_message = "Database file name cannot be empty.";
    return false;
  }
  if (options.result_file_name.IsEmpty()) {
    error_message = "Result file name cannot be empty.";
    return false;
  }

  options.output_directory = EnsureTrailingSlash(options.output_directory);
  return true;
}

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

#if defined(_WIN32) || defined(_WIN64)
static WString output_path = WString::FromLocal("../output_files/");
static WString input_path = WString::FromLocal("../input_files/fulltextsearch/");
#else
static WString output_path = WString::FromLocal("./output_files/");
static WString input_path = WString::FromLocal("./input_files/fulltextsearch/");
#endif

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

class SearchCallbackImp:public SearchCallback
{
public:
  SearchCallbackImp(const WString& output_txt_file_path)
    : file_(NULL) {
    String output_file = String::FromUnicode(output_txt_file_path);
#if defined(_WIN32) || defined(_WIN64)
    fopen_s(&file_, output_file, "wb");
#else
    file_ = fopen((const char*)output_file, "wb");
#endif
    if (!file_)
      throw Exception(__FILE__, __LINE__, __FUNCTION__, foxit::e_ErrFile);
  }

  virtual ~SearchCallbackImp() {
    if (file_ != NULL) {
      fclose(file_);
      file_ = NULL;
    }
  }

  virtual void Release() {delete this;}
  virtual int RetrieveSearchResult(const wchar_t* filePath, int pageIndex, const WString& matchResult, int matchStartTextIndex, int matchEndTextIndex, const WString& sentence, int sentenceStartTextIndex, int sentenceEndTextIndex);

private:
  FILE* file_;
};

int SearchCallbackImp::RetrieveSearchResult(const wchar_t* filePath, int pageIndex, const WString& matchResult, int matchStartTextIndex, int matchEndTextIndex, const WString& sentence, int sentenceStartTextIndex, int sentenceEndTextIndex) {
  String result_str;
  result_str.Format("RetrieveSearchResult:\nFound file is: %ls\nPage index is: %d\nStart text index: %d\nEnd text index: %d\nMatch is: %ls\nSentence start index:%d\nSentence end index:%d\nSentence is: %ls\n", filePath, pageIndex, matchStartTextIndex, matchEndTextIndex, (const wchar_t*)matchResult, sentenceStartTextIndex, sentenceEndTextIndex, (const wchar_t*)sentence);
  if (file_ != NULL) {
    fwrite((const char*)result_str, sizeof(char), result_str.GetLength(), file_);
    static const char line_break[] = "\r\n";
    fwrite(line_break, sizeof(char), 2, file_);
  }

  return 0;
}

class Search_Pause : public PauseCallback
{
public:
  Search_Pause(int pause_count_limit = 0, bool always_pause = false)
    :pause_count_(0)
    ,pause_count_limit_(pause_count_limit)
    ,always_pause_(always_pause)
  {

  }

  virtual FX_BOOL NeedToPauseNow()
  {
    if (always_pause_) return true;
    if (pause_count_< pause_count_limit_)
    {
      pause_count_ ++;
      return 1;
    }
    else{
      pause_count_ = 0;
      return 0; // This is to test a case: valid PauseCallback but needParseNow() will always return FALSE.
    }
  }

  void  ClearCount()
  {
    pause_count_ = 0;
  }

private:
  int pause_count_limit_;
  int pause_count_;
  bool always_pause_;
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

  WString output_directory = options.output_directory;
#if defined(_WIN32) || defined(_WIN64)
  _mkdir(String::FromUnicode(output_directory));
#else
  mkdir(String::FromUnicode(output_directory), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

  WString db_path = output_directory + options.db_file_name;
  WString result_path = output_directory + options.result_file_name;
  if (FileExists(result_path)) {
    cout << "Result file already exists (no-overwrite): " << String::FromUnicode(result_path) << endl;
    return 1;
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }
  try
  {
    FullTextSearch full_text_search;
    //The path of data base to store the indexed data...
    String dbPath = String::FromUnicode(db_path);
    full_text_search.SetDataBasePath(dbPath);
    // Get document source information.
    DocumentsSource docs_source(String::FromUnicode(options.input_directory));
    // Create a Pause callback object implemented by users to pause the updating process.
    Search_Pause pause(options.pause_limit);
    Progressive search_progress = full_text_search.StartUpdateIndex(docs_source, &pause, false);
    int state = Progressive::e_ToBeContinued;
    while(state == Progressive::e_ToBeContinued)
    {
      state = search_progress.Continue();
    }
    // Create a callback object which will be invoked when a matched one is found.
    SearchCallbackImp* searchCallback = new SearchCallbackImp(result_path);
    // Search the specified keyword from the indexed data source.
    full_text_search.SearchOf(options.keyword, FullTextSearch::e_RankHitCountASC, searchCallback);

	cout << "FullTextSearch demo." << endl;
  }
   catch(...)
  {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}