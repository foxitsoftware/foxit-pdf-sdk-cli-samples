// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to get preflight keys and analyze or fixup PDF file.

#include <iostream>
#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/addon/compliance/fs_compliancecommon.h"
#include "../../../include/addon/compliance/fs_pdfa.h"
#include "../../../include/addon/compliance/fs_pdfcompliance.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace addon;
using namespace compliance;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs+E1TwQ1TvEjdYeW3tAF/BwfSNSqQ==";
static const char* key = "ezJvj18mtBp399sXJVWqfHfLliq+pf9v7BPcYPMEPpxz159rzwHa2YFF3YlZHukjsTx1EnsRivFv8AKWWYt1VYVP0FFFK9ZNfK807Wy4D3KhfxFHkjhFaOaEpt3c3iruJRjXvjPGzqPW96741oNr5ypv5klt2seAvt9wnfKKyniUdWC3dCuuUyCOUVCBi0D+T71Oa1VDr1nx/X/EjMJcso7TP+dZGMprZP1LYr/OfcyZ7V6wkP5S16tfvSUuSl8J0Us0MBa0edHfbdME6c1cf7Za78Ut5Kyp6AHVHkUqhnPqhMYmQVX3FXLhQGj7kbNr3t2kuBLIAdilIqcRMnjFgtV362ePhKj/Ed3sCt/afdVRPF9y2A9BRAYgfoYJ9i9TlYkOjmnUbZt+hI5x2kuBUNpZ4uRaKw0em1Y0FeEdJG9qzv0eN2I+0k7f7bUjVu3GRyffKNfvUC+n/SSjhXMRxzowQNfXDcKq51gWzLZpubpcz0RPEe1foBoHHmAD61AQ5YX/DnOGhAME3XhYoz6R76C9UH6kOeiLsfJGMGUoz3IkKLvVC+3CCSR//k7mLc6FaKlriJUaIfLu3sNj7dzsHku295Qqx3FyPM86o8WQE9wQ4skW5DZzrbKM2k0C/8oQIn+hX9aYjR2lIh5sNMaRUlrT3k+wZnF6jZG4Wnef+ruVZ9MCoTqqAJ/C9dlHeha8LblZsQ7cEuQhQAjcILeL3U/HEidcyvOTpjLTZaaR8ECUz1U/58YjONl5bQGK+CRwImlWoTVf0WEfe9I5TMpWtzzgkJ1flh6MXzt+PY5kVdxzYpqyvQbrridjYd44/+d6/7ZNgEIXiHBgil9/y3W2FgrzC1rgMgM630NX/cxOwHtgo15sOkwSlZQA89KNknQm9gVLvRFcG1skXHQ7L4KCSrOba62fi705ucgcj6lJh72fpXNqhetiKuIKmtGK+MSX1ztYJV2ymHue9TZWNkfl6mBV5VbAhrCqSnK82W0OkVNkbv2khSNDbQM5E87AhZOj34v2f45wTp2ssZZSI/eD9K8OKZ1zqhA61tYm8apKrhZJ9hdoYxfYFM1rFhcEARB+7IrPGZKxaeOzXmatKu65/m8zeYxvDdOXptc+3TFU5NgmsJU095yK+FDD2psy5Bep6ZG+/TH/FAqWncjjO7TIbGPfmsKfmI26UNAgC1bh2/+pgQGsfdoIEoboY3LZzzCUQ1tAzf8yOxc5+Sl54Fw=";

struct PreflightCommand {
  WString action;           // list, analyze, fixup
  WString input_file;       // input PDF path (required for analyze/fixup)
  WString output_dir;       // output directory (required for analyze/fixup)
  WString preflight_key;    // preflight key (required for analyze/fixup)
  WString resource_path;    // compliance resource folder path (required for analyze/fixup)
  WString unlock_code;      // compliance engine unlock code (optional, default: "")
  WString library_type;     // foxit | standards | prepress (default: foxit)
  WString operate_type;     // profiles | checks | fixups (default: profiles)
  WString report_format;    // pdf | xml | txt | html (default: pdf)
  bool show_help;

  PreflightCommand()
      : library_type(L"foxit"),
        operate_type(L"profiles"),
        report_format(L"pdf"),
        show_help(false) {}
};

void PrintUsage() {
  cout << "Usage:" << endl;
  cout << "preflight --action <list|analyze|fixup> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --action <type>                 Action: list | analyze | fixup." << endl;
  cout << "  --input <path>                  Input PDF file path (required for analyze/fixup)." << endl;
  cout << "  --output <dir>                  Output directory (required for analyze/fixup)." << endl;
  cout << "  --key <preflight-key>           Preflight key, e.g. pppp_ConverttoPDFA1a (required for analyze/fixup)." << endl;
  cout << "  --resource-path <dir>           Compliance resource folder path (required for analyze/fixup)." << endl << endl;
  cout << "Optional:" << endl;
  cout << "  --library-type <type>           Library type for list/analyze/fixup: foxit | standards | prepress. Default: foxit." << endl;
  cout << "  --operate-type <type>           Operate type: profiles | checks | fixups. Default: profiles." << endl;
  cout << "  --report-format <type>          Report file format: pdf | xml | txt | html. Default: pdf." << endl;
  cout << "  --unlock-code <code>            Compliance engine unlock code. Default: empty." << endl;
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

bool ParseCommand(int argc, char* argv[], PreflightCommand& command) {
  if (argc <= 1) {
    PrintUsage();
    return false;
  }

  bool has_action = false;
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
    if (key.Equal("--action") || key.Equal("-a")) {
      if (!value.Equal("list") && !value.Equal("analyze") && !value.Equal("fixup")) {
        printf("Invalid action: %s (must be list, analyze, or fixup)\n", (const char*)value);
        return false;
      }
      command.action = WString::FromUTF8(value);
      has_action = true;
    } else if (key.Equal("--input") || key.Equal("-i")) {
      command.input_file = WString::FromUTF8(value);
    } else if (key.Equal("--output") || key.Equal("-o")) {
      command.output_dir = WString::FromUTF8(value);
    } else if (key.Equal("--key") || key.Equal("-k")) {
      command.preflight_key = WString::FromUTF8(value);
    } else if (key.Equal("--resource-path")) {
      command.resource_path = WString::FromUTF8(value);
    } else if (key.Equal("--unlock-code")) {
      command.unlock_code = WString::FromUTF8(value);
    } else if (key.Equal("--library-type")) {
      if (!value.Equal("foxit") && !value.Equal("standards") && !value.Equal("prepress")) {
        printf("Invalid library-type: %s (must be foxit, standards, or prepress)\n", (const char*)value);
        return false;
      }
      command.library_type = WString::FromUTF8(value);
    } else if (key.Equal("--operate-type")) {
      if (!value.Equal("profiles") && !value.Equal("checks") && !value.Equal("fixups")) {
        printf("Invalid operate-type: %s (must be profiles, checks, or fixups)\n", (const char*)value);
        return false;
      }
      command.operate_type = WString::FromUTF8(value);
    } else if (key.Equal("--report-format")) {
      if (!value.Equal("pdf") && !value.Equal("xml") && !value.Equal("txt") && !value.Equal("html")) {
        printf("Invalid report-format: %s (must be pdf, xml, txt, or html)\n", (const char*)value);
        return false;
      }
      command.report_format = WString::FromUTF8(value);
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (!has_action) {
    printf("--action is required.\n");
    PrintUsage();
    return false;
  }

  if (!command.action.Equal(L"list")) {
    if (command.input_file.IsEmpty()) {
      printf("--input is required for analyze/fixup action.\n");
      PrintUsage();
      return false;
    }
    if (!FileExists(command.input_file)) {
      printf("Input file does not exist: %s\n", (const char*)String::FromUnicode(command.input_file));
      return false;
    }
    if (command.output_dir.IsEmpty()) {
      printf("--output is required for analyze/fixup action.\n");
      PrintUsage();
      return false;
    }
    if (command.preflight_key.IsEmpty()) {
      printf("--key is required for analyze/fixup action.\n");
      PrintUsage();
      return false;
    }
    if (command.resource_path.IsEmpty()) {
      printf("--resource-path is required for analyze/fixup action.\n");
      PrintUsage();
      return false;
    }
  }

  return true;
}

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

class TextDoc {
public:
  TextDoc(const String& file_name, const String& fill_mode);
  TextDoc(const WString& file_name, const WString& fill_mode);
  ~TextDoc();

  void Write(const char* text_content);
  void Write(const wchar_t* text_content);

private:
  FILE* file_;
};

TextDoc::TextDoc(const String& file_name, const String& file_mode) throw(Exception) : file_(NULL) {
#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file_, file_name, (const char*)file_mode);
#else
  file_ = fopen((const char*)file_name, (const char*)file_mode);
#endif

  if (!file_)
    throw Exception(__FILE__, __LINE__, __FUNCTION__, foxit::e_ErrFile);

  uint8 temp[] = {0xFF,0xFE};
  fwrite(temp, sizeof(uint8), 2, file_);
  fseek(file_, 0, SEEK_END);

}

TextDoc::TextDoc(const WString& file_name, const WString& file_mode) throw(Exception)  : file_(NULL) {
  String s_file_name = String::FromUnicode(file_name);
  String s_file_mode = String::FromUnicode(file_mode);

#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file_, (const char*)s_file_name, (const char*)s_file_mode);
#else
  file_ = fopen((const char*)s_file_name, (const char*)s_file_mode);
#endif
  if (!file_)
    throw Exception(__FILE__, __LINE__, __FUNCTION__, foxit::e_ErrFile);

  uint8 temp[] = {0xFF,0xFE};
  fwrite(temp, sizeof(uint8), 2, file_);
  fseek(file_, 0, SEEK_END);

}

TextDoc::~TextDoc() {
  fclose(file_);
  file_ = NULL;
}

void TextDoc::Write(const char* text_content) {
  WString wide_str = WString::FromLocal(text_content);
  Write(wide_str);
}

void TextDoc::Write(const wchar_t* text_content) {
  WString wide_str(text_content);
  if (wide_str.IsEmpty()) return ;
  String utf16le_str = wide_str.UTF16LE_Encode(false);
  if (utf16le_str.IsEmpty()) return;
  int length = utf16le_str.GetLength();
  fwrite((const char*)utf16le_str, sizeof(char), length, file_);
}

class MyComplianceProgressCallback : public ProgressCallback {
public:
  MyComplianceProgressCallback(const WString& output_txt_file_path);
  ~MyComplianceProgressCallback();

  virtual void Release();
  virtual void UpdateCurrentStateData(int current_rate, const WString& current_state_string);

private:
  TextDoc text_doc_;
};

MyComplianceProgressCallback::MyComplianceProgressCallback(const WString& output_txt_file_path)
  : text_doc_(output_txt_file_path, L"w+b") {}

MyComplianceProgressCallback::~MyComplianceProgressCallback() {}

void MyComplianceProgressCallback::Release() {

}

void MyComplianceProgressCallback::UpdateCurrentStateData(int current_rate, const WString& current_state_string) {
  std::cout << "Current rate:" << current_rate << std::endl;

  char temp_string[32];
#if defined(_WIN32) || defined(_WIN64)
  sprintf_s(temp_string, 32, "Current rate:%d, state str:", current_rate);
#else
  sprintf(temp_string, "Current rate:%d, state str:", current_rate);
#endif  // defined(_WIN32) || defined(_WIN64)
  text_doc_.Write(temp_string);
  text_doc_.Write(current_state_string);
  text_doc_.Write("\r\n");
}

void ListPreflightKeys(Preflight preflight, const PreflightSettingData::LibraryType& library_type, const PreflightSettingData::OperateType& operate_type) {
  StringArray group_arr = preflight.GetGroupNamesArray(library_type, operate_type);
  for (int i = 0; i < group_arr.GetSize(); i++) {
    cout << "Group name: " << group_arr[i] << endl;
    StringArray item_array = preflight.GetItemKeysArray(group_arr[i]);
    cout << "    Item count: " << item_array.GetSize() << endl;
    for (int j = 0; j < item_array.GetSize(); j++) {
      cout << "    Item name: " << item_array[j] << endl;
    }
  }
}

void AnalyzeAndFixup(Preflight preflight, const PreflightSettingData::OperateType& operate_type, const WString& preflight_key, bool is_fixed, const WString& input_file, const WString& output_dir, const WString& input_basename) {
  PreflightSettingData setting_data;
  setting_data.operate_type = operate_type;
  setting_data.preflight_key = preflight_key;
  setting_data.first_page_index = 0;
  setting_data.src_pdf_path = input_file;
  WString fixpath = output_dir + input_basename + L"_" + preflight_key + L"_fixed.pdf";
  setting_data.saved_pdf_path = fixpath;
  MyComplianceProgressCallback* progress_callback = new MyComplianceProgressCallback(output_dir + input_basename + L"_" + preflight_key + L"_progress.txt");
  if(is_fixed)
    preflight.AnalyzeAndFixup(setting_data, progress_callback);
  else
    preflight.Analyze(setting_data, progress_callback);
}

void GenerateReport(Preflight preflight, const PreflightReportSettingData::ReportFileFormatType& format_type, const WString& preflight_key, const WString& output_dir, const WString& input_basename) {
  PreflightReportSettingData report_setting_data;
  report_setting_data.report_file_format_type = format_type;
  // Choose extension matching format
  WString ext;
  if (format_type == PreflightReportSettingData::e_ReportFileFormatTypeXml) ext = L".xml";
  else if (format_type == PreflightReportSettingData::e_ReportFileFormatTypeTxt) ext = L".txt";
  else if (format_type == PreflightReportSettingData::e_ReportFileFormatTypeHtml) ext = L".html";
  else ext = L".pdf";
  WString path = output_dir + input_basename + L"_" + preflight_key + L"_report" + ext;
  report_setting_data.report_file_path = path;
  report_setting_data.to_generate_overview = true;
  report_setting_data.to_highlight_problems = true;
  report_setting_data.problems_highlight_method = PreflightReportSettingData::e_ProblemsHighlightMethodTransparentMasks;
  preflight.GenerateReport(report_setting_data, NULL);
}

int main(int argc, char *argv[])
{
  PreflightCommand command;
  if (!ParseCommand(argc, argv, command)) return 1;

#if defined(_WIN32) || defined(_WIN64)
  if (!command.output_dir.IsEmpty())
    _mkdir(String::FromUnicode(command.output_dir));
#else
  if (!command.output_dir.IsEmpty())
    mkdir(String::FromUnicode(command.output_dir), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    // Initialize compliance engine.
    ErrorCode error_code = ComplianceEngine::Initialize(command.resource_path, (const char*)String::FromUnicode(command.unlock_code));
    if (error_code != foxit::e_ErrSuccess) {
      switch (error_code) {
      case foxit::e_ErrInvalidLicense:
        printf("[Failed] Compliance module is not contained in current Foxit PDF SDK keys.\n");
        break;
      default:
        printf("[Failed] Fail to initialize compliance engine. Error: %d\n", error_code);
        break;
      }
      return 1;
    }
    cout << "ComplianceEngine is initialized." << endl;
    {
    Preflight preflight;

      // Resolve library_type, operate_type, report_format from command strings
      PreflightSettingData::LibraryType lib_type = PreflightSettingData::e_LibraryTypeFoxit;
      if (command.library_type.Equal(L"standards")) lib_type = PreflightSettingData::e_LibraryTypePDFStandards;
      else if (command.library_type.Equal(L"prepress")) lib_type = PreflightSettingData::e_LibraryTypePrepressColorTransparency;

      PreflightSettingData::OperateType op_type = PreflightSettingData::e_OperateTypeProfiles;
      if (command.operate_type.Equal(L"checks")) op_type = PreflightSettingData::e_OperateTypeSingleChecks;
      else if (command.operate_type.Equal(L"fixups")) op_type = PreflightSettingData::e_OperateTypeSingleFixups;

      PreflightReportSettingData::ReportFileFormatType rpt_format = PreflightReportSettingData::e_ReportFileFormatTypePdf;
      if (command.report_format.Equal(L"xml")) rpt_format = PreflightReportSettingData::e_ReportFileFormatTypeXml;
      else if (command.report_format.Equal(L"txt")) rpt_format = PreflightReportSettingData::e_ReportFileFormatTypeTxt;
      else if (command.report_format.Equal(L"html")) rpt_format = PreflightReportSettingData::e_ReportFileFormatTypeHtml;

      // Derive basename from input file (strip directory and .pdf extension)
      WString input_basename;
      {
        WString p = command.input_file;
        int last_sep = -1;
        for (int ci = 0; ci < (int)p.GetLength(); ci++) {
          wchar_t ch = p.GetAt(ci);
          if (ch == L'/' || ch == L'\\') last_sep = ci;
        }
        WString fname = (last_sep >= 0) ? p.Mid(last_sep + 1) : p;
        // Strip trailing .pdf if present
        if (fname.GetLength() > 4) {
          WString tail = fname.Mid(fname.GetLength() - 4);
          // case-insensitive compare manually
          String tail_s = String::FromUnicode(tail);
          String lower;
          for (int ci = 0; ci < tail_s.GetLength(); ci++) {
            char c = tail_s.GetAt(ci);
            lower += String((char)(c >= 'A' && c <= 'Z' ? c + 32 : c));
          }
          if (lower.Equal(".pdf"))
            fname = fname.Mid(0, fname.GetLength() - 4);
        }
        input_basename = fname;
      }

      if (command.action.Equal(L"list")) {
        // List preflight keys with the specified library type and operate type.
        ListPreflightKeys(preflight, lib_type, op_type);
      } else if (command.action.Equal(L"analyze")) {
        cout << "== Analyze. ==" << endl;
        AnalyzeAndFixup(preflight, op_type, command.preflight_key, false, command.input_file, command.output_dir, input_basename);
        GenerateReport(preflight, rpt_format, command.preflight_key, command.output_dir, input_basename);
      } else if (command.action.Equal(L"fixup")) {
        cout << "== Fixup. ==" << endl;
        bool can_fixup = preflight.CanFixup(String::FromUnicode(command.preflight_key));
        if (!can_fixup) {
          printf("The preflight key does not support fixup: %s\n", (const char*)String::FromUnicode(command.preflight_key));
        } else {
          AnalyzeAndFixup(preflight, op_type, command.preflight_key, true, command.input_file, command.output_dir, input_basename);
          GenerateReport(preflight, rpt_format, command.preflight_key, command.output_dir, input_basename);
        }
      }
    }
    // Release compliance engine.
    ComplianceEngine::Release();

    cout << "== End: Preflight demo. ==" << endl;

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

