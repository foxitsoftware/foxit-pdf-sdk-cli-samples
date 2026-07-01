// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to search text in all pages in PDF document.

// Include Foxit SDK header files.
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

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

struct SearchCommand {
  WString input_file;
  WString output_file;
  WString pattern;
  bool match_case;
  bool match_whole_word;
  bool consecutive;
  int start_page;
  int end_page;
  bool show_help;

  SearchCommand()
      : match_case(false),
        match_whole_word(false),
        consecutive(false),
        start_page(0),
        end_page(-1),
        show_help(false) {}
};

void PrintUsage() {
  cout << "Usage:" << endl;
  cout << "search --input <input.pdf> --output <output.txt> --pattern <text> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --input <path>                  Input PDF file path." << endl;
  cout << "  --output <path>                 Output txt file path." << endl;
  cout << "  --pattern <text>                Search pattern." << endl << endl;
  cout << "Options:" << endl;
  cout << "  --match-case                    Match case. Default: false." << endl;
  cout << "  --match-whole-word              Match whole word. Default: false." << endl;
  cout << "  --consecutive                   Search consecutive. Default: false." << endl;
  cout << "  --start-page <int>              Start page index (0-based). Default: 0." << endl;
  cout << "  --end-page <int>                End page index (0-based, -1 = last page). Default: -1." << endl;
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

bool ParseCommand(int argc, char* argv[], SearchCommand& command) {
  if (argc <= 1) {
    PrintUsage();
    return false;
  }

  bool has_input = false;
  bool has_output = false;
  bool has_pattern = false;
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
    if (key.Equal("--consecutive")) {
      command.consecutive = true;
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
    } else if (key.Equal("--start-page")) {
      command.start_page = atoi((const char*)value);
    } else if (key.Equal("--end-page")) {
      command.end_page = atoi((const char*)value);
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (!has_input || !has_output || !has_pattern) {
    printf("--input, --output, and --pattern are all required.\n");
    PrintUsage();
    return false;
  }
  if (!FileExists(command.input_file)) {
    printf("Input file does not exist: %s\n", (const char*)String::FromUnicode(command.input_file));
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

class TextOutput{
public:
  TextOutput(const String& file_name, const String& fill_mode);
  TextOutput(const WString& file_name, const WString& fill_mode);
  TextOutput() : file_(NULL){};
  ~TextOutput();

  void Write(const char* format);
  void Write(const wchar_t* text_content);

private:
  FILE* file_;
};

TextOutput::TextOutput(const String& file_name, const String& file_mode) throw(Exception) : file_(NULL) {
#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file_, file_name, (const char*)file_mode);
#else
  file_ = fopen((const char*)file_name, (const char*)file_mode);
#endif

  if (!file_)
    throw Exception(__FILE__, __LINE__, __FUNCTION__, foxit::e_ErrFile);

  uint8 temp[] = { 0xFF, 0xFE };
  fwrite(temp, sizeof(uint8), 2, file_);
  fseek(file_, 0, SEEK_END);
}

TextOutput::TextOutput(const WString& file_name, const WString& file_mode) throw(Exception)  : file_(NULL) {
  String s_file_name = String::FromUnicode(file_name);
  String s_file_mode = String::FromUnicode(file_mode);

#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file_, (const char*)s_file_name, (const char*)s_file_mode);
#else
  file_ = fopen((const char*)s_file_name, (const char*)s_file_mode);
#endif
  if (!file_)
    throw Exception(__FILE__, __LINE__, __FUNCTION__, foxit::e_ErrFile);

  uint8 temp[] = { 0xFF, 0xFE };
  fwrite(temp, sizeof(uint8), 2, file_);
  fseek(file_, 0, SEEK_END);
}

TextOutput::~TextOutput() {
  if(file_)
    fclose(file_);
  file_ = NULL;
}

void TextOutput::Write(const char* text_content) {
  WString wide_str = WString::FromLocal(text_content);
  Write((FX_LPCWSTR)wide_str);
}

void TextOutput::Write(const wchar_t* text_content) {
  WString wide_str(text_content);
  if (wide_str.IsEmpty())
    return;
  String utf16le_str = wide_str.UTF16LE_Encode(false);
  if (utf16le_str.IsEmpty())
    return;
  wcout << (FX_LPCWSTR)wide_str;
  int length = utf16le_str.GetLength();
  fwrite((const char*)utf16le_str, sizeof(char), length, file_);
}

void OutputMatchedInfo(TextOutput& text_out, TextSearch search, int matched_index) {
  int page_index = search.GetMatchPageIndex();
  WString format_str = L"";
  format_str.Format(L"Index of matched pattern:\t%d\r\n", matched_index);
  text_out.Write((FX_LPCWSTR)format_str);
  format_str.Format(L"\tpage:\t%d\r\n", page_index);
  text_out.Write((FX_LPCWSTR)format_str);
  format_str.Format(L"\tmatch char start index:\t%d\r\n", search.GetMatchStartCharIndex());
  text_out.Write((FX_LPCWSTR)format_str);
  format_str.Format(L"\tmatch char end index:\t%d\r\n", search.GetMatchEndCharIndex());
  text_out.Write((FX_LPCWSTR)format_str);
  format_str.Format(L"\tmatch sentence start index:\t%d\r\n", search.GetMatchSentenceStartIndex());
  text_out.Write((FX_LPCWSTR)format_str);
  format_str = L"\tmatch sentence:\t";
  format_str += search.GetMatchSentence();
  format_str += L"\r\n";
  text_out.Write((FX_LPCWSTR)format_str);
  RectFArray rect_array = search.GetMatchRects();
  int rect_count = rect_array.GetSize();
  format_str.Format(L"\tmatch rectangles count:\t%d\r\n", rect_count);
  text_out.Write((FX_LPCWSTR)format_str);
  for (int i = 0; i < rect_count; i++) {
    foxit::RectF rect = rect_array[i];
    format_str.Format(L"\trectangle(in PDF space) :%d\t[left = %.4f, bottom = %.4f, right = %.4f, top = %.4f]\r\n", i,
      rect.left, rect.bottom, rect.right, rect.top);
    text_out.Write((FX_LPCWSTR)format_str);
  }
}

int main(int argc, char *argv[])
{
  SearchCommand command;
  if (!ParseCommand(argc, argv, command)) return 1;

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    PDFDoc doc(command.input_file);
    ErrorCode error_code = doc.Load();
    if (error_code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(command.input_file), error_code);
      return 1;
    }
    TextOutput text_out(command.output_file, L"wb+");

    // Search for all pages of doc.
    TextSearch search(doc, NULL);

    int page_count = doc.GetPageCount();
    int start_index = command.start_page;
    int end_index = (command.end_page < 0) ? page_count - 1 : command.end_page;
    search.SetStartPage(start_index);
    search.SetEndPage(end_index);

    search.SetPattern(command.pattern);

    foxit::uint32 flags = TextSearch::e_SearchNormal;
    if (command.match_case)
      flags |= TextSearch::e_SearchMatchCase;
    if (command.match_whole_word)
      flags |= TextSearch::e_SearchMatchWholeWord;
    if (command.consecutive)
      flags |= TextSearch::e_SearchConsecutive;
    search.SetSearchFlags(flags);

    WString format_str = L"";
    format_str = L"Begin search ";
    format_str += command.pattern;
    format_str += L" at ";
    format_str += command.input_file;
    format_str += L".\n";
    text_out.Write((FX_LPCWSTR)format_str);
    format_str.Format(L"Start index:\t%d\r\n", start_index);
    text_out.Write((FX_LPCWSTR)format_str);
    format_str.Format(L"End index:\t%d\r\n", end_index);
    text_out.Write((FX_LPCWSTR)format_str);
    format_str = L"Match key:\t";
    format_str += command.pattern;
    format_str += L"\r\n";
    text_out.Write((FX_LPCWSTR)format_str);
    WString match_case = flags & TextSearch::e_SearchMatchCase ? L"Yes" : L"No";
    format_str = L"Match Case\t";
    format_str += match_case;
    format_str += L"\r\n";
    text_out.Write((FX_LPCWSTR)format_str);
    WString match_whole_word = flags & TextSearch::e_SearchMatchWholeWord ? L"Yes" : L"No";
    format_str = L"Match whole word:\t";
    format_str += match_whole_word;
    format_str += L"\r\n";
    text_out.Write((FX_LPCWSTR)format_str);
    WString match_consecutive = flags & TextSearch::e_SearchConsecutive ? L"Yes" : L"No";
    format_str = L"Consecutive:\t";
    format_str += match_consecutive;
    format_str += L"\r\n";
    text_out.Write((FX_LPCWSTR)format_str);
    int match_count = 0;
    while (search.FindNext()) {
      RectFArray rect_array = search.GetMatchRects();
      OutputMatchedInfo(text_out, search, match_count);
      match_count ++;
    }
    wcout << L"Matched " << match_count << L" counts." << endl;

    wcout << L"Search demo finished." << endl;

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

