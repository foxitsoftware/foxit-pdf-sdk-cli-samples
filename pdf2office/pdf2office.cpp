// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF Conversion SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to convert PDF files to Office(Word,
// Excel or PowerPoint) format files.

#if defined(_WIN32) || defined(_WIN64) || defined(__linux__)

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#endif
#include <iostream>
#include <string>
#include <sstream>
#include <cctype>

#include "../../../include/common/fs_common.h"
#include "../../../include/addon/conversion/pdf2office/fs_pdf2office.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using namespace foxit::common::file;
using foxit::common::Library;
using namespace addon::conversion::pdf2office;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9kxu1NwuFssENxZIFiUiyIciAwog==";
static const char* key = "ezKXjt3HvBh39LusL0W0ja4nKga7YWN7L9uNJpJgDMZz1JNbmcLa2b1tqsmx19y6TQPBVqRI/VmE/86WZfydwx784Mrzfy/zZCQafLxRbv65cVqbliME6uSUBBEC5ah5HmZCiPjTa8jxPLfR5bU54Z84/FFx6N4a0Dkz8y5vaDGS/qKv0EieNMfUeY7NKOrkgly/tHIuT+n6BzsvfD+hLG5a51d3dFhj6CxYgW8wCjATdwDnNg1OZW2g9ChLZYkPUoJp/iu9cOz2LtIVXOKtrRh17Uo4D+8G8mkxIvWTy5wf2iO2q0dpxFR9z1Bg2sDN7sTAGzwpBO1+bupJu0HZixpjZ3IFcyhcVfN6fpBNB/MB/RbMk8S0tr1k28SIMvd+AaEk3OiZznTtpADKCs1C9hF0HOBZCYgICfw3cLJmTRuQhjChiefH2Ba6US3qSSh1flDGS4jnC+y0wq2EHnOgt3VussC/eP1gbnnftf9wK4GPhxe9bKmKD/ZH+zkdY1DKkWU2xyewsZDiYirseVLCVMpYjMHeTe+qSMdH50JMWC9rNq5gGdIQspNTS8gUcGtNvMu1Sytltq2BvI4DLAQ8rU/u25PM2NC9MMz7jpplsiq2JCdYc8SKJ4TTqg2juEGzB/tw0XNRZQJGR05QGKHnce81v+aQYXqTffaud8EhwPHnF7BY1r8L+Kc6+gKmuyVYU8Ik92WorWbP3n+3m2/qp3rGiU6J/2e7obWUI3IBTHx/NZrcQ5wM3pqtZrJ8/yhnFSXW1qjQyx0rJK+EgL/S7oxYTO6LcSCeCoP30uxdx95nHPvpTevbDc7Et4oXr+f9WqFh8/nYEAngW+ytdGL2Idr3i3h6K8vJC1DRjXA6ieSt4B79ppjZA5OUZXA1V1tFgyC5dDtDTw8A3u7U7tyS+vCzKZptLChaPPRVE/zpT8kHsN05n79z010JqgmZgQp/PjlBoQk9rBV/wH3KTKEbQTGr7jOTG4UyC580dVDCWcHdolmMr4dk/60pA8aj0xnulEnDkynbE9JWqyFRUQUWDbWNbk/f9ip5FRbkCLwMxVPYs6vLOyAlW0KnVQbDPGt0vjqamqO7ikuu8Lmd0fDTQZvXY41GwJ/9yXzsY+vyWyj2r/SNDSnqyNLs/VTZUItL4XAU1XVcZRl/el9DdVAT9p9G1hOmhEy52c2AOHeSzIcrvYjILRLSXuBIltQ9VluPju3jyn4eFWw0/I9lnmMotAib5dCQ5LAD8BpwBG4oDm78Gk8THf3j";

struct Pdf2OfficeCommand {
  WString input_file;
  WString output_file;
  String format;
  WString library_path;
  WString engine_path;

  // PDF2OfficeSettingData parameters
  bool enable_ml_recognition;
  bool include_pdf_comments;
  bool enable_trailing_space;
  bool include_images;
  int timeout;
  bool enable_matching_system_fonts;

  // PDF2WordSettingData parameters
  bool word_enable_retain_page_layout;
  bool word_enable_generate_headers_and_footers;
  bool word_enable_generate_footnotes_and_endnotes;
  bool word_enable_generate_page_rendered_break;
  int word_max_blank_paragraphs_per_page_bottom;

  // PDF2PowerPointSettingData parameters
  bool ppt_enable_aggressively_split_sections;
  bool ppt_enable_adapt_to_largest_page;

  // PDF2ExcelSettingData parameters
  String excel_decimal_symbol;
  String excel_thousands_separator;
  int excel_workbook_settings;  // 0=document, 1=each_table, 2=each_page
  bool excel_enable_aggressive_table_repair;
  bool excel_include_watermarks;

  bool show_help;

  Pdf2OfficeCommand()
      : format("word"),
        library_path(L""),
        engine_path(L""),
        enable_ml_recognition(false),
        include_pdf_comments(true),
        enable_trailing_space(true),
        include_images(true),
        timeout(0),
        enable_matching_system_fonts(false),
        word_enable_retain_page_layout(false),
        word_enable_generate_headers_and_footers(true),
        word_enable_generate_footnotes_and_endnotes(false),
        word_enable_generate_page_rendered_break(false),
        word_max_blank_paragraphs_per_page_bottom(-1),
        ppt_enable_aggressively_split_sections(false),
        ppt_enable_adapt_to_largest_page(false),
        excel_decimal_symbol(""),
        excel_thousands_separator(""),
        excel_workbook_settings(2),  // e_WorkbookSettingsEachPage
        excel_enable_aggressive_table_repair(true),
        excel_include_watermarks(false),
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
  cout << "pdf2office --input <input.pdf> --output <output_file> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --input <path>                           Input PDF path." << endl;
  cout << "  --output <path>                          Output file path (e.g. output.docx)." << endl << endl;
  cout << "Optional:" << endl;
  cout << "  --format <word|excel|ppt>                Output format. Aliases: docx/xlsx/pptx (default: word)." << endl;
  cout << "  --library-path <path>                    Foxit Conversion SDK library path." << endl;
  cout << "  --engine-path <path>                     Engine path (for LibreOffice on Linux)." << endl << endl;
  cout << "PDF2OfficeSettingData options:" << endl;
  cout << "  --enable-ml-recognition <true|false>     Enable ML-based table recognition (default: false)." << endl;
  cout << "  --include-pdf-comments <true|false>      Include PDF comments (default: true)." << endl;
  cout << "  --enable-trailing-space <true|false>     Add trailing spaces (default: true)." << endl;
  cout << "  --include-images <true|false>            Include images (default: true)." << endl;
  cout << "  --timeout <int>                          Conversion timeout in milliseconds (default: 0)." << endl;
  cout << "  --enable-matching-system-fonts <true|false>  Match system fonts (default: false)." << endl << endl;
  cout << "Word options:" << endl;
  cout << "  --word-retain-layout <true|false>        Retain page layout (default: false)." << endl;
  cout << "  --word-headers-footers <true|false>      Generate headers/footers (default: true)." << endl;
  cout << "  --word-footnotes <true|false>            Generate footnotes/endnotes (default: false)." << endl;
  cout << "  --word-page-break <true|false>           Generate page break markers (default: false)." << endl;
  cout << "  --word-max-blanks <int>                  Max blank paragraphs at page bottom (default: -1)." << endl << endl;
  cout << "PowerPoint options:" << endl;
  cout << "  --ppt-split-sections <true|false>        Split sections aggressively (default: false)." << endl;
  cout << "  --ppt-adapt-pages <true|false>           Adapt to largest page (default: false)." << endl << endl;
  cout << "Excel options:" << endl;
  cout << "  --excel-decimal <string>                 Decimal symbol (default: empty)." << endl;
  cout << "  --excel-thousands <string>               Thousands separator (default: empty)." << endl;
  cout << "  --excel-workbook <0|1|2>                 Workbook setting: 0=document, 1=each_table, 2=each_page (default: 2)." << endl;
  cout << "  --excel-table-repair <true|false>        Aggressive table repair (default: true)." << endl;
  cout << "  --excel-watermarks <true|false>          Include watermarks (default: false)." << endl << endl;
  cout << "  --help                                   Show this message." << endl;
}

bool CanonicalizeFormat(const String& raw_format, String& canonical_format) {
  string text = (const char*)raw_format;
  for (size_t i = 0; i < text.size(); ++i) {
    text[i] = (char)tolower((unsigned char)text[i]);
  }
  if (text == "word" || text == "docx") {
    canonical_format = "word";
    return true;
  }
  if (text == "excel" || text == "xlsx") {
    canonical_format = "excel";
    return true;
  }
  if (text == "ppt" || text == "pptx" || text == "powerpoint") {
    canonical_format = "ppt";
    return true;
  }
  return false;
}

bool ParseBoolValue(const String& value, bool& out_value) {
  string text = (const char*)value;
  for (size_t i = 0; i < text.size(); ++i) {
    text[i] = (char)tolower((unsigned char)text[i]);
  }
  if (text == "true" || text == "1" || text == "yes") {
    out_value = true;
    return true;
  }
  if (text == "false" || text == "0" || text == "no") {
    out_value = false;
    return true;
  }
  return false;
}

bool ParseIntValue(const String& value, int& out_value) {
  char* end_ptr = NULL;
  out_value = static_cast<int>(strtol((const char*)value, &end_ptr, 10));
  return end_ptr != NULL && *end_ptr == '\0';
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

bool ParseCommand(int argc, char* argv[], Pdf2OfficeCommand& command) {
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
    } else if (key.Equal("--format") || key.Equal("-t")) {
      command.format = value;
    } else if (key.Equal("--library-path")) {
      command.library_path = WString::FromUTF8(value);
    } else if (key.Equal("--engine-path")) {
      command.engine_path = WString::FromUTF8(value);
    } else if (key.Equal("--enable-ml-recognition")) {
      if (!ParseBoolValue(value, command.enable_ml_recognition)) return false;
    } else if (key.Equal("--include-pdf-comments")) {
      if (!ParseBoolValue(value, command.include_pdf_comments)) return false;
    } else if (key.Equal("--enable-trailing-space")) {
      if (!ParseBoolValue(value, command.enable_trailing_space)) return false;
    } else if (key.Equal("--include-images")) {
      if (!ParseBoolValue(value, command.include_images)) return false;
    } else if (key.Equal("--timeout")) {
      if (!ParseIntValue(value, command.timeout)) return false;
    } else if (key.Equal("--enable-matching-system-fonts")) {
      if (!ParseBoolValue(value, command.enable_matching_system_fonts)) return false;
    } else if (key.Equal("--word-retain-layout")) {
      if (!ParseBoolValue(value, command.word_enable_retain_page_layout)) return false;
    } else if (key.Equal("--word-headers-footers")) {
      if (!ParseBoolValue(value, command.word_enable_generate_headers_and_footers)) return false;
    } else if (key.Equal("--word-footnotes")) {
      if (!ParseBoolValue(value, command.word_enable_generate_footnotes_and_endnotes)) return false;
    } else if (key.Equal("--word-page-break")) {
      if (!ParseBoolValue(value, command.word_enable_generate_page_rendered_break)) return false;
    } else if (key.Equal("--word-max-blanks")) {
      if (!ParseIntValue(value, command.word_max_blank_paragraphs_per_page_bottom)) return false;
    } else if (key.Equal("--ppt-split-sections")) {
      if (!ParseBoolValue(value, command.ppt_enable_aggressively_split_sections)) return false;
    } else if (key.Equal("--ppt-adapt-pages")) {
      if (!ParseBoolValue(value, command.ppt_enable_adapt_to_largest_page)) return false;
    } else if (key.Equal("--excel-decimal")) {
      command.excel_decimal_symbol = value;
    } else if (key.Equal("--excel-thousands")) {
      command.excel_thousands_separator = value;
    } else if (key.Equal("--excel-workbook")) {
      if (!ParseIntValue(value, command.excel_workbook_settings)) return false;
      if (command.excel_workbook_settings < 0 || command.excel_workbook_settings > 2) {
        printf("--excel-workbook must be 0, 1, or 2\n");
        return false;
      }
    } else if (key.Equal("--excel-table-repair")) {
      if (!ParseBoolValue(value, command.excel_enable_aggressive_table_repair)) return false;
    } else if (key.Equal("--excel-watermarks")) {
      if (!ParseBoolValue(value, command.excel_include_watermarks)) return false;
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

  String canonical_format;
  if (!CanonicalizeFormat(command.format, canonical_format)) {
    printf("Unsupported --format value: %s\n", (const char*)command.format);
    return false;
  }

  command.format = canonical_format;
  return true;
}

class CustomConvertCallback : public ConvertCallback {
 public:
  CustomConvertCallback() {}
  ~CustomConvertCallback() {}
  virtual bool NeedToPause() {
    return true;
  }

  virtual void ProgressNotify(int converted_count, int total_count) {}
};

class FileReader : public ReaderCallback {
 public:
  FileReader()
      : file_(NULL){}
  ~FileReader() {}

  bool LoadFile(const char* file_path) {
#if defined(_WIN32) || defined(_WIN64)
    fopen_s(&file_, file_path, "rb");
#else
    file_ = fopen(file_path, "rb");
#endif
    if (!file_)
      return false;
    return true;
  }

  FX_FILESIZE GetSize() {
    if (!file_)
      return 0;
    fseek(file_, 0, SEEK_END);
    return (FX_FILESIZE)ftell(file_);
  }

  FX_BOOL ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) {
    if (!file_)
      return 0;
    if (0 != fseek(file_, (long)offset, 0))
      return 0;
    if (0 == fread(buffer, size, 1, file_))
      return 0;
    return 1;
  }

  size_t ReadBlock(void* buffer, size_t size) {
    if (!file_)
      return false;
    if (0 != fseek(file_, 0, 0))
      return 0;
    return fread(buffer, size, 1, file_);
  }

  void Release() {
    if (file_)
      fclose(file_);
    file_ = NULL;
    delete this;
  }

 private:
  FILE* file_;
};

class FileStream : public StreamCallback {
 public:
  FileStream()
      : file_(NULL)
      , ref_(1)
      , cur_pos_(SEEK_SET) {}

  ~FileStream() {}

  bool LoadFile(const char* file_path) {
#if defined(_WIN32) || defined(_WIN64)
    fopen_s(&file_, file_path, "wb");
#else
    file_ = fopen(file_path, "wb");
#endif
    if (!file_)
      return false;
    return true;
  }

  FX_FILESIZE GetSize() {
    if (!file_)
      return 0;
    fseek(file_, 0, SEEK_END);
    return (FX_FILESIZE)ftell(file_);
  }

  FX_BOOL Flush() {
    fflush(file_);
    return true;
  }

  FX_FILESIZE GetPosition() {
    return cur_pos_;
  }

  FX_BOOL ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) {
    if (!file_)
      return false;
    if (0 != fseek(file_, (long)offset, 0))
        return false;
    if (0 == fread(buffer, size, sizeof(char), file_))
        return false;

    cur_pos_ = offset + size;
    return true;
  }

  size_t ReadBlock(void* buffer, size_t size) {
    if (ReadBlock(buffer, GetSize(), size))
      return size;
    else
      return 0;
  }

  FX_BOOL WriteBlock(const void* buffer, FX_FILESIZE offset, size_t size) {
    if (!file_)
      return false;
    fseek(file_, (long)offset, SEEK_SET);
    uint64 write_size = fwrite(buffer, sizeof(char), size, file_);
    if (write_size == size) {
      cur_pos_ = offset + size;
      return true;
    }
    return false;
  }

  FX_BOOL WriteBlock(const void* data, size_t size) {
    return WriteBlock(data, GetSize(), size);
  }

  FileStream* Retain() {
    ref_++;
    return this;
  }

  FX_BOOL IsEOF() {
    if (!file_)
      return 0;
    fseek(file_, 0, SEEK_END);
    if (cur_pos_ < (FX_FILESIZE)ftell(file_))
      return false;
    return true;
  }

  void Release() {
    ref_--;
    if (ref_ == 0) {
      if (file_)
        fclose(file_);
      file_ = NULL;
      delete this;
    }
  }

 private:
  FILE* file_;
  int ref_;
  FX_FILESIZE cur_pos_;
};

int main(int argc, char* argv[]) {
  Pdf2OfficeCommand command;
  if (!ParseCommand(argc, argv, command)) {
    return command.show_help ? 0 : 1;
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    if (foxit::e_ErrInvalidLicense == error_code)
      cout << "[Failed] Current used Foxit PDF Conversion SDK key information is invalid." << endl;
    return 1;
  }

  try {
    PDF2Office::Initialize(command.library_path, command.engine_path);
    
    // Configure PDF2WordSettingData
    PDF2WordSettingData word_setting;
    word_setting.enable_retain_page_layout = command.word_enable_retain_page_layout;
    word_setting.enable_generate_headers_and_footers = command.word_enable_generate_headers_and_footers;
    word_setting.enable_generate_footnotes_and_endnotes = command.word_enable_generate_footnotes_and_endnotes;
    word_setting.enable_generate_page_rendered_break = command.word_enable_generate_page_rendered_break;
    word_setting.max_blank_paragraphs_per_page_bottom = command.word_max_blank_paragraphs_per_page_bottom;
    
    // Configure PDF2PowerPointSettingData
    PDF2PowerPointSettingData ppt_setting(
      command.ppt_enable_aggressively_split_sections,
      command.ppt_enable_adapt_to_largest_page
    );
    
    // Configure PDF2ExcelSettingData
    PDF2ExcelSettingData excel_setting(
      command.excel_decimal_symbol,
      command.excel_thousands_separator,
      static_cast<PDF2ExcelSettingData::WorkbookSettings>(command.excel_workbook_settings),
      command.excel_enable_aggressive_table_repair,
      command.excel_include_watermarks
    );
    
    // Configure PDF2OfficeSettingData
    PDF2OfficeSettingData setting_data(
      L"",  // metrics_data_folder_path (deprecated)
      command.enable_ml_recognition,
      foxit::common::Range(),  // all pages
      command.include_pdf_comments,
      word_setting,
      ppt_setting,
      excel_setting,
      command.enable_trailing_space,
      command.include_images,
      command.timeout,
      command.enable_matching_system_fonts
    );

    CustomConvertCallback callback;
    Progressive progressive;
    if (command.format.Equal("word")) {
      progressive = PDF2Office::StartConvertToWord(command.input_file, NULL, command.output_file, setting_data, &callback);
      cout << "Convert PDF file to Word format file." << endl;
    } else if (command.format.Equal("excel")) {
      progressive = PDF2Office::StartConvertToExcel(command.input_file, NULL, command.output_file, setting_data);
      cout << "Convert PDF file to Excel format file." << endl;
    } else {
      progressive = PDF2Office::StartConvertToPowerPoint(command.input_file, NULL, command.output_file, setting_data);
      cout << "Convert PDF file to PowerPoint format file." << endl;
    }

    if (progressive.GetRateOfProgress() != 100) {
      Progressive::State state = Progressive::e_ToBeContinued;
      while (Progressive::e_ToBeContinued == state) {
        state = progressive.Continue();
      }
    }
    cout << "Output file: " << (const char*)String::FromUnicode(command.output_file) << endl;
  } catch (const Exception& e) {
    switch (e.GetErrCode()) {
      case foxit::e_ErrNoPDF2OfficeModuleRight:
        cout << "[Failed] Conversion module is not contained in current Foxit PDF Conversion SDK keys." << endl;
        break;
      default:
        cout << (FX_LPCSTR)e.GetMessage() << endl;
        break;
    }
    err_ret = 1;
  } catch (...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }
  PDF2Office::Release();
  return err_ret;
}
#endif  // #if defined(_WIN32) || defined(_WIN64) || defined(__linux__)
