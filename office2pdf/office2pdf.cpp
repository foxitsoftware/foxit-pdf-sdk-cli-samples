// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to convert Word or Excel files to PDF files.

#if !defined(__APPLE__)

// Include Foxit SDK header files.
#if defined(_WIN32) || defined(_WIN64)
#include<string.h>
#include<direct.h>
#else
#include <sys/stat.h>
#endif  // #if defined(_WIN32) || defined(_WIN64)

#include <cstdlib>
#include "../../../include/common/fs_common.h"
#include "../../../include/addon/conversion/fs_convert.h"
#include "../../../include/addon/conversion/office2pdf/fs_office2pdf.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9kxu1NwuFssENxZIFiUiyIciAwog==";
static const char* key = "ezKXjt3HvBh39LusL0W0ja4nKga7YWN7L9uNJpJgDMZz1JNbmcLa2b1tqsmx19y6TQPBVqRI/VmE/86WZfydwx784Mrzfy/zZCQafLxRbv65cVqbliME6uSUBBEC5ah5HmZCiPjTa8jxPLfR5bU54Z84/FFx6N4a0Dkz8y5vaDGS/qKv0EieNMfUeY7NKOrkgly/tHIuT+n6BzsvfD+hLG5a51d3dFhj6CxYgW8wCjATdwDnNg1OZW2g9ChLZYkPUoJp/iu9cOz2LtIVXOKtrRh17Uo4D+8G8mkxIvWTy5wf2iO2q0dpxFR9z1Bg2sDN7sTAGzwpBO1+bupJu0HZixpjZ3IFcyhcVfN6fpBNB/MB/RbMk8S0tr1k28SIMvd+AaEk3OiZznTtpADKCs1C9hF0HOBZCYgICfw3cLJmTRuQhjChiefH2Ba6US3qSSh1flDGS4jnC+y0wq2EHnOgt3VussC/eP1gbnnftf9wK4GPhxe9bKmKD/ZH+zkdY1DKkWU2xyewsZDiYirseVLCVMpYjMHeTe+qSMdH50JMWC9rNq5gGdIQspNTS8gUcGtNvMu1Sytltq2BvI4DLAQ8rU/u25PM2NC9MMz7jpplsiq2JCdYc8SKJ4TTqg2juEGzB/tw0XNRZQJGR05QGKHnce81v+aQYXqTffaud8EhwPHnF7BY1r8L+Kc6+gKmuyVYU8Ik92WorWbP3n+3m2/qp3rGiU6J/2e7obWUI3IBTHx/NZrcQ5wM3pqtZrJ8/yhnFSXW1qjQyx0rJK+EgL/S7oxYTO6LcSCeCoP30uxdx95nHPvpTevbDc7Et4oXr+f9WqFh8/nYEAngW+ytdGL2Idr3i3h6K8vJC1DRjXA6ieSt4B79ppjZA5OUZXA1V1tFgyC5dDtDTw8A3u7U7tyS+vCzKZptLChaPPRVE/zpT8kHsN05n79z010JqgmZgQp/PjlBoQk9rBV/wH3KTKEbQTGr7jOTG4UyC580dVDCWcHdolmMr4dk/60pA8aj0xnulEnDkynbE9JWqyFRUQUWDbWNbk/f9ip5FRbkCLwMxVPYs6vLOyAlW0KnVQbDPGt0vjqamqO7ikuu8Lmd0fDTQZvXY41GwJ/9yXzsY+vyWyj2r/SNDSnqyNLs/VTZUItL4XAU1XVcZRl/el9DdVAT9p9G1hOmhEy52c2AOHeSzIcrvYjILRLSXuBIltQ9VluPju3jyn4eFWw0/I9lnmMotAib5dCQ5LAD8BpwBG4oDm78Gk8THf3j";

struct Office2PdfCommand {
  WString input_file;
  WString output_file;
  WString password;
  String convert_type;
  String engine;  // "third-party" (default) or "foxit"

  // Third-party engine options
  bool word_include_doc_props;
  int word_optimize_option;
  int word_content_option;
  int word_bookmark_option;
  bool word_convert_to_pdfa;

  bool excel_include_doc_props;
  int excel_quality;
  bool excel_ignore_print_area;
  int excel_scale_type;
  bool excel_convert_to_pdfa;

  int ppt_intent;
  bool ppt_frame_output_slides;
  int ppt_output_type;
  int ppt_handout_order;
  bool ppt_output_hidden_slides;
  bool ppt_include_doc_props;

  // Linux third-party engine path
  WString engine_path;

  // Foxit engine (Office2PDF module) options
  WString foxit_lib_path;
  WString foxit_res_path;
  bool foxit_embed_font;
  bool foxit_word_generate_bookmark;
  bool foxit_excel_separate_workbook;
  bool foxit_excel_output_hidden_sheets;

  bool has_input;
  bool has_output;
  bool has_type;

  Office2PdfCommand()
      : password(L""),
        engine("third-party"),
        word_include_doc_props(false),
        word_optimize_option(0),
        word_content_option(0),
        word_bookmark_option(0),
        word_convert_to_pdfa(false),
        excel_include_doc_props(false),
        excel_quality(0),
        excel_ignore_print_area(true),
        excel_scale_type(0),
        excel_convert_to_pdfa(false),
        ppt_intent(0),
        ppt_frame_output_slides(false),
        ppt_output_type(foxit::addon::conversion::PowerPoint2PDFSettingData::e_OutputSlides),
        ppt_handout_order(0),
        ppt_output_hidden_slides(false),
        ppt_include_doc_props(false),
        foxit_embed_font(false),
        foxit_word_generate_bookmark(false),
        foxit_excel_separate_workbook(false),
        foxit_excel_output_hidden_sheets(false),
        has_input(false),
        has_output(false),
        has_type(false) {}
};

class SdkLibMgr {
 public:
  SdkLibMgr() : is_initialize_(false) {};
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
  cout << "Usage:" << endl
      << "office2pdf_xxx --input <office path> --output <pdf path> --type <word|excel|ppt> [options]" << endl
      << endl
      << "Required:" << endl
      << "--input <path>                   Input office file path." << endl
      << "--output <path>                  Output pdf file path." << endl
      << "--type <word|excel|ppt>          Conversion type." << endl
      << endl
      << "Common optional:" << endl
      << "--password <text>                Source file password." << endl
      << "--engine <third-party|foxit>     Conversion engine to use (default: third-party)." << endl
      << "--engine-path <path>             Path to the third-party engine (e.g. /usr/lib/libreoffice/program). Linux only." << endl
      << endl
      << "Foxit engine options (requires --engine foxit, Windows/Linux x86/x64 only):" << endl
      << "--foxit-lib-path <path>          Path of Foxit Conversion SDK library folder." << endl
      << "--foxit-res-path <path>          Path of resource data folder (required for foxit engine)." << endl
      << "--foxit-embed-font <true|false>  Embed fonts in converted PDF (default: false)." << endl
      << "--foxit-word-generate-bookmark <true|false>  Generate bookmarks from Word (default: false)." << endl
      << "--foxit-excel-separate-workbook <true|false> Export each worksheet separately (default: false)." << endl
      << "--foxit-excel-output-hidden-sheets <true|false> Output hidden worksheets (default: false)." << endl
      << endl
      << "Third-party engine Word optional:" << endl
      << "--word-include-doc-props <true|false>" << endl
      << "--word-optimize-option <int>     SDK enum value.(0-1)" << endl
      << "--word-content-option <int>      SDK enum value.(0-1)" << endl
      << "--word-bookmark-option <int>     SDK enum value.(0-2)" << endl
      << "--word-convert-to-pdfa <true|false>" << endl
      << endl
      << "Third-party engine Excel optional:" << endl
      << "--excel-include-doc-props <true|false>" << endl
      << "--excel-quality <int>            SDK enum value.(0-1)" << endl
      << "--excel-ignore-print-area <true|false>" << endl
      << "--excel-scale-type <int>         SDK enum value.(0-3)" << endl
      << "--excel-convert-to-pdfa <true|false>" << endl
      << endl
      << "Third-party engine PowerPoint optional:" << endl
      << "--ppt-intent <int>               SDK enum value.(0-1)" << endl
      << "--ppt-frame-output-slides <true|false>" << endl
      << "--ppt-output-type <int>          SDK enum value.(1-9)" << endl
      << "--ppt-handout-order <int>        SDK enum value.(0-1)" << endl
      << "--ppt-output-hidden-slides <true|false>" << endl
      << "--ppt-include-doc-props <true|false>" << endl;
}

bool ParseBoolValue(const String& value, bool& out_value) {
  if (value.Equal("true") || value.Equal("1")) {
    out_value = true;
    return true;
  }
  if (value.Equal("false") || value.Equal("0")) {
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

bool IsSupportedType(const String& value) {
  return value.Equal("word") || value.Equal("excel") || value.Equal("ppt");
}

bool AnalysisParameter(int argc, char* argv[], Office2PdfCommand& command) {
  if (argc < 7 || ((argc - 1) % 2 != 0)) {
    return false;
  }

  for (int i = 1; i < argc; i += 2) {
    String key = String(argv[i]);
    String value = String(argv[i + 1]);

    if (key.Equal("--input")) {
      command.input_file = WString::FromUTF8(value);
      command.has_input = true;
    } else if (key.Equal("--output")) {
      command.output_file = WString::FromUTF8(value);
      command.has_output = true;
    } else if (key.Equal("--type")) {
      if (!IsSupportedType(value)) {
        return false;
      }
      command.convert_type = value;
      command.has_type = true;
    } else if (key.Equal("--password")) {
      command.password = WString::FromUTF8(value);
    } else if (key.Equal("--engine")) {
      if (!value.Equal("third-party") && !value.Equal("foxit")) {
        return false;
      }
      command.engine = value;
    } else if (key.Equal("--engine-path")) {
      command.engine_path = WString::FromUTF8(value);
    } else if (key.Equal("--foxit-lib-path")) {
      command.foxit_lib_path = WString::FromUTF8(value);
    } else if (key.Equal("--foxit-res-path")) {
      command.foxit_res_path = WString::FromUTF8(value);
    } else if (key.Equal("--foxit-embed-font")) {
      if (!ParseBoolValue(value, command.foxit_embed_font)) return false;
    } else if (key.Equal("--foxit-word-generate-bookmark")) {
      if (!ParseBoolValue(value, command.foxit_word_generate_bookmark)) return false;
    } else if (key.Equal("--foxit-excel-separate-workbook")) {
      if (!ParseBoolValue(value, command.foxit_excel_separate_workbook)) return false;
    } else if (key.Equal("--foxit-excel-output-hidden-sheets")) {
      if (!ParseBoolValue(value, command.foxit_excel_output_hidden_sheets)) return false;
    } else if (key.Equal("--word-include-doc-props")) {
      if (!ParseBoolValue(value, command.word_include_doc_props)) {
        return false;
      }
    } else if (key.Equal("--word-optimize-option")) {
      if (!ParseIntValue(value, command.word_optimize_option)) {
        return false;
      }
    } else if (key.Equal("--word-content-option")) {
      if (!ParseIntValue(value, command.word_content_option)) {
        return false;
      }
    } else if (key.Equal("--word-bookmark-option")) {
      if (!ParseIntValue(value, command.word_bookmark_option)) {
        return false;
      }
    } else if (key.Equal("--word-convert-to-pdfa")) {
      if (!ParseBoolValue(value, command.word_convert_to_pdfa)) {
        return false;
      }
    } else if (key.Equal("--excel-include-doc-props")) {
      if (!ParseBoolValue(value, command.excel_include_doc_props)) {
        return false;
      }
    } else if (key.Equal("--excel-quality")) {
      if (!ParseIntValue(value, command.excel_quality)) {
        return false;
      }
    } else if (key.Equal("--excel-ignore-print-area")) {
      if (!ParseBoolValue(value, command.excel_ignore_print_area)) {
        return false;
      }
    } else if (key.Equal("--excel-scale-type")) {
      if (!ParseIntValue(value, command.excel_scale_type)) {
        return false;
      }
    } else if (key.Equal("--excel-convert-to-pdfa")) {
      if (!ParseBoolValue(value, command.excel_convert_to_pdfa)) {
        return false;
      }
    } else if (key.Equal("--ppt-intent")) {
      if (!ParseIntValue(value, command.ppt_intent)) {
        return false;
      }
    } else if (key.Equal("--ppt-frame-output-slides")) {
      if (!ParseBoolValue(value, command.ppt_frame_output_slides)) {
        return false;
      }
    } else if (key.Equal("--ppt-output-type")) {
      if (!ParseIntValue(value, command.ppt_output_type)) {
        return false;
      }
    } else if (key.Equal("--ppt-handout-order")) {
      if (!ParseIntValue(value, command.ppt_handout_order)) {
        return false;
      }
    } else if (key.Equal("--ppt-output-hidden-slides")) {
      if (!ParseBoolValue(value, command.ppt_output_hidden_slides)) {
        return false;
      }
    } else if (key.Equal("--ppt-include-doc-props")) {
      if (!ParseBoolValue(value, command.ppt_include_doc_props)) {
        return false;
      }
    } else {
      return false;
    }
  }

  return command.has_input && command.has_output && command.has_type;
}


int main(int argc, char *argv[]) {
  if ((argc > 1 && String(argv[1]).Equal("--help")) || argc < 2) {
    PrintUsage();
    return 0;
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    if (foxit::e_ErrInvalidLicense == error_code)
      cout << "[Failed] Current used Foxit PDF SDK key information is invalid." << endl;
    return 1;
  }
  Office2PdfCommand command;
  if (!AnalysisParameter(argc, argv, command)) {
    PrintUsage();
    return 1;
  }

  if (!command.output_file.IsEmpty()) {
    String output_path_utf8 = String::FromUnicode(command.output_file);
    if (output_path_utf8.Find("/") < 0 && output_path_utf8.Find("\\") < 0) {
      cout << "Output path should contain a file name with path information." << endl;
      return 1;
    }
  }

  try {
    if (command.engine.Equal("foxit")) {
#if defined(_WIN32) || defined(_WIN64) || (defined(__linux__) && (defined(__i386__) || defined(__x86_64__)))
      if (command.foxit_res_path.IsEmpty()) {
        cout << "--foxit-res-path is required when using foxit engine." << endl;
        return 1;
      }
      foxit::addon::conversion::office2pdf::Office2PDF::Initialize(command.foxit_lib_path);
      foxit::addon::conversion::office2pdf::Office2PDFSettingData office2pdf_setting_data;
      office2pdf_setting_data.resource_folder_path = command.foxit_res_path;
      office2pdf_setting_data.is_embed_font = command.foxit_embed_font;
      office2pdf_setting_data.word_setting_data.is_generate_bookmark = command.foxit_word_generate_bookmark;
      office2pdf_setting_data.excel_setting_data.is_separate_workbook = command.foxit_excel_separate_workbook;
      office2pdf_setting_data.excel_setting_data.is_output_hidden_worksheets = command.foxit_excel_output_hidden_sheets;
      if (command.convert_type.Equal("word")) {
        foxit::addon::conversion::office2pdf::Office2PDF::ConvertFromWord(command.input_file, command.password, command.output_file, office2pdf_setting_data);
        cout << "Convert Word file to PDF file (Foxit engine)." << endl;
      } else if (command.convert_type.Equal("excel")) {
        foxit::addon::conversion::office2pdf::Office2PDF::ConvertFromExcel(command.input_file, command.password, command.output_file, office2pdf_setting_data);
        cout << "Convert Excel file to PDF file (Foxit engine)." << endl;
      } else if (command.convert_type.Equal("ppt")) {
        foxit::addon::conversion::office2pdf::Office2PDF::ConvertFromPowerPoint(command.input_file, command.password, command.output_file, office2pdf_setting_data);
        cout << "Convert PowerPoint file to PDF file (Foxit engine)." << endl;
      } else {
        cout << "Unknown conversion type: " << command.convert_type << endl;
        err_ret = 1;
      }
      foxit::addon::conversion::office2pdf::Office2PDF::Release();
#else
      cout << "Foxit engine is only supported on Windows and Linux x86/x64." << endl;
      return 1;
#endif
    } else if (command.convert_type.Equal("word")) {
      foxit::addon::conversion::Word2PDFSettingData word_convert_setting_data;
      word_convert_setting_data.include_doc_props = command.word_include_doc_props;
      word_convert_setting_data.optimize_option = static_cast<foxit::addon::conversion::Word2PDFSettingData::ConvertOptimizeOption>(command.word_optimize_option);
      word_convert_setting_data.content_option = static_cast<foxit::addon::conversion::Word2PDFSettingData::ConvertContentOption>(command.word_content_option);
      word_convert_setting_data.bookmark_option = static_cast<foxit::addon::conversion::Word2PDFSettingData::ConvertBookmarkOption>(command.word_bookmark_option);
      word_convert_setting_data.convert_to_pdfa = command.word_convert_to_pdfa;
#if defined(_WIN32) || defined(_WIN64)
      foxit::addon::conversion::Convert::FromWord(command.input_file, command.password, command.output_file, word_convert_setting_data);
#else
      foxit::addon::conversion::Convert::FromWord(command.input_file, command.password, command.output_file, command.engine_path, word_convert_setting_data);
#endif
      cout << "Convert Word file to PDF file." << endl;
    } else if (command.convert_type.Equal("excel")) {
      foxit::addon::conversion::Excel2PDFSettingData excel_convert_setting_data;
      excel_convert_setting_data.include_doc_props = command.excel_include_doc_props;
      excel_convert_setting_data.quality = static_cast<foxit::addon::conversion::Excel2PDFSettingData::ConvertQuality>(command.excel_quality);
      excel_convert_setting_data.ignore_print_area = command.excel_ignore_print_area;
      excel_convert_setting_data.scale_type = static_cast<foxit::addon::conversion::Excel2PDFSettingData::ScaleType>(command.excel_scale_type);
      excel_convert_setting_data.convert_to_pdfa = command.excel_convert_to_pdfa;
#if defined(_WIN32) || defined(_WIN64)
      foxit::addon::conversion::Convert::FromExcel(command.input_file, command.password, command.output_file, excel_convert_setting_data);
#else
      foxit::addon::conversion::Convert::FromExcel(command.input_file, command.password, command.output_file, command.engine_path, excel_convert_setting_data);
#endif
      cout << "Convert Excel file to PDF file." << endl;
    } else if (command.convert_type.Equal("ppt")) {
      foxit::addon::conversion::PowerPoint2PDFSettingData ppt_convert_setting_data;
      ppt_convert_setting_data.intent = static_cast<foxit::addon::conversion::PowerPoint2PDFSettingData::ConvertIntent>(command.ppt_intent);
      ppt_convert_setting_data.frame_output_slides = command.ppt_frame_output_slides;
      ppt_convert_setting_data.output_type = static_cast<foxit::addon::conversion::PowerPoint2PDFSettingData::OutputType>(command.ppt_output_type);
      ppt_convert_setting_data.handout_order = static_cast<foxit::addon::conversion::PowerPoint2PDFSettingData::HandoutOrder>(command.ppt_handout_order);
      ppt_convert_setting_data.output_hidden_slides = command.ppt_output_hidden_slides;
      ppt_convert_setting_data.include_doc_props = command.ppt_include_doc_props;
#if defined(_WIN32) || defined(_WIN64)
      foxit::addon::conversion::Convert::FromPowerPoint(command.input_file, command.password, command.output_file, ppt_convert_setting_data);
#else
      foxit::addon::conversion::Convert::FromPowerPoint(command.input_file, command.password, command.output_file, command.engine_path, ppt_convert_setting_data);
#endif
      cout << "Convert PowerPoint file to PDF file." << endl;
    } else {
      cout << "Unknown conversion type: " << command.convert_type << endl;
      err_ret = 1;
    }
  } catch (const Exception& e) {
    switch (e.GetErrCode()) {
      case foxit::e_ErrNoConversionModuleRight:
        cout << "[Failed] Conversion module is not contained in current Foxit PDF SDK keys." << endl;
        break;
      case foxit::e_ErrNoMicroOfficeInstalled:
        cout << "[Failed] No Microsoft Office is installed in current system, so fail to do conversion from Word/Excel/PowerPoint to PDF." << endl;
        break;
      case foxit::e_ErrNoOffice2PDFModuleRight:
        cout << "[Failed] Office2PDF module is not contained in current Foxit PDF SDK keys." << endl;
        break;
      default:
        cout << e.GetMessage() << endl;
        break;
    }
    err_ret = 1;
  } catch (...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}
#endif  // #if !defined(__APPLE__)

