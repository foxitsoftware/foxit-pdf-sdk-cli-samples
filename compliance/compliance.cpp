// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to verify if a PDF file is PDFA-1a version
// or convert a PDF file to PDFA-1a version.

#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/addon/compliance/fs_compliancecommon.h"
#include "../../../include/addon/compliance/fs_pdfa.h"
#include "../../../include/addon/compliance/fs_pdfcompliance.h"
#include "../../../include/addon/compliance/fs_pdfx.h"
#include "../../../include/addon/compliance/fs_pdfe.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace addon;
using namespace compliance;

enum ComplianceType {
  e_ComplianceTypeUnknown = 0,
  e_ComplianceTypePDFA,
  e_ComplianceTypePDFX,
  e_ComplianceTypePDFE,
};

struct CliOptions {
  std::string input_file;
  std::string output_file;
  std::string type_name;
  std::string resource_dir;
  bool show_help;

  CliOptions() : show_help(false) {}
};

static std::string ToUpper(const std::string& text) {
  std::string upper = text;
  for (size_t i = 0; i < upper.size(); ++i) {
    upper[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(upper[i])));
  }
  return upper;
}

static ComplianceType ParseComplianceType(const std::string& raw_type) {
  std::string type = ToUpper(raw_type);
  if (type == "PDFA") return e_ComplianceTypePDFA;
  if (type == "PDFX") return e_ComplianceTypePDFX;
  if (type == "PDFE") return e_ComplianceTypePDFE;
  return e_ComplianceTypeUnknown;
}

static void PrintUsage() {
  cout
    << "Usage: compliance -i <input.pdf> -o <output.pdf> --type <PDFA|PDFX|PDFE> --resource-dir <path>" << endl
    << endl
    << "Required options:" << endl
    << "  -i <file>                Input PDF path" << endl
    << "  -o <file>                Output PDF path" << endl
    << "  --type <name>            Compliance target type: PDFA | PDFX | PDFE" << endl
    << "  --resource-dir <path>    Compliance resource folder path" << endl
    << endl
    << "Optional options:" << endl
    << "  --help                   Show this help message" << endl
    << endl
    << "Examples:" << endl
    << "  compliance -i ./input/AboutFoxit.pdf -o ./output/about_pdfa.pdf --type PDFA --resource-dir ./compliance_resources" << endl
    << "  compliance -i ./input/AboutFoxit.pdf -o ./output/about_pdfe.pdf --type PDFE --resource-dir ./compliance_resources" << endl;
}

static bool ParseArgs(int argc, char* argv[], CliOptions& options) {
  for (int i = 1; i < argc; ++i) {
    String arg(argv[i]);
    if (arg.Equal("--help")) {
      options.show_help = true;
      return true;
    }

    if (arg.Equal("-i") || arg.Equal("-o") || arg.Equal("--type") || arg.Equal("--resource-dir")) {
      if (i + 1 >= argc) {
        printf("Missing value for argument: %s\n", argv[i]);
        return false;
      }
      const char* value = argv[++i];
      if (arg.Equal("-i")) {
        options.input_file = value;
      } else if (arg.Equal("-o")) {
        options.output_file = value;
      } else if (arg.Equal("--type")) {
        options.type_name = value;
      } else {
        options.resource_dir = value;
      }
    } else {
      printf("Unknown argument: %s\n", argv[i]);
      return false;
    }
  }
  return true;
}

static std::string GetParentDir(const std::string& path) {
  size_t pos = path.find_last_of("/\\");
  if (pos == std::string::npos) return "";
  return path.substr(0, pos);
}

static void EnsureOutputDir(const std::string& dir_path) {
  if (dir_path.empty()) return;

  std::string normalized = dir_path;
  for (size_t i = 0; i < normalized.size(); ++i) {
    if (normalized[i] == '\\') normalized[i] = '/';
  }

  std::string current;
  size_t start = 0;
  if (normalized.size() > 1 && normalized[1] == ':') {
    current = normalized.substr(0, 2);
    start = 2;
  }
  if (start < normalized.size() && normalized[start] == '/') {
    current += '/';
    ++start;
  }

  while (start < normalized.size()) {
    size_t sep = normalized.find('/', start);
    std::string token = (sep == std::string::npos)
      ? normalized.substr(start)
      : normalized.substr(start, sep - start);

    if (!token.empty()) {
      if (!current.empty() && current[current.size() - 1] != '/') current += '/';
      current += token;
#if defined(_WIN32) || defined(_WIN64)
      _mkdir(current.c_str());
#else
      mkdir(current.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    }

    if (sep == std::string::npos) break;
    start = sep + 1;
  }
}

static bool ValidateArgs(const CliOptions& options) {
  if (options.input_file.empty()) {
    printf("Error: -i is required.\n");
    return false;
  }
  if (options.output_file.empty()) {
    printf("Error: -o is required.\n");
    return false;
  }
  if (options.type_name.empty()) {
    printf("Error: --type is required.\n");
    return false;
  }
  if (options.resource_dir.empty()) {
    printf("Error: --resource-dir is required.\n");
    return false;
  }
  if (ParseComplianceType(options.type_name) == e_ComplianceTypeUnknown) {
    printf("Error: unsupported --type value '%s'. Supported types: PDFA, PDFX, PDFE.\n", options.type_name.c_str());
    return false;
  }

  std::ifstream input_test(options.input_file.c_str(), std::ios::binary);
  if (!input_test.good()) {
    printf("Error: input file does not exist or cannot be opened: %s\n", options.input_file.c_str());
    return false;
  }

  std::ifstream output_test(options.output_file.c_str(), std::ios::binary);
  if (output_test.good()) {
    printf("Error: output file already exists: %s. Please provide a new output file path.\n", options.output_file.c_str());
    return false;
  }

  return true;
}

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs+E1TwQ1TvEjdYeW3tAF/BwfSNSqQ==";
static const char* key = "ezJvj18mtBp399sXJVWqfHfLliq+pf9v7BPcYPMEPpxz159rzwHa2YFF3YlZHukjsTx1EnsRivFv8AKWWYt1VYVP0FFFK9ZNfK807Wy4D3KhfxFHkjhFaOaEpt3c3iruJRjXvjPGzqPW96741oNr5ypv5klt2seAvt9wnfKKyniUdWC3dCuuUyCOUVCBi0D+T71Oa1VDr1nx/X/EjMJcso7TP+dZGMprZP1LYr/OfcyZ7V6wkP5S16tfvSUuSl8J0Us0MBa0edHfbdME6c1cf7Za78Ut5Kyp6AHVHkUqhnPqhMYmQVX3FXLhQGj7kbNr3t2kuBLIAdilIqcRMnjFgtV362ePhKj/Ed3sCt/afdVRPF9y2A9BRAYgfoYJ9i9TlYkOjmnUbZt+hI5x2kuBUNpZ4uRaKw0em1Y0FeEdJG9qzv0eN2I+0k7f7bUjVu3GRyffKNfvUC+n/SSjhXMRxzowQNfXDcKq51gWzLZpubpcz0RPEe1foBoHHmAD61AQ5YX/DnOGhAME3XhYoz6R76C9UH6kOeiLsfJGMGUoz3IkKLvVC+3CCSR//k7mLc6FaKlriJUaIfLu3sNj7dzsHku295Qqx3FyPM86o8WQE9wQ4skW5DZzrbKM2k0C/8oQIn+hX9aYjR2lIh5sNMaRUlrT3k+wZnF6jZG4Wnef+ruVZ9MCoTqqAJ/C9dlHeha8LblZsQ7cEuQhQAjcILeL3U/HEidcyvOTpjLTZaaR8ECUz1U/58YjONl5bQGK+CRwImlWoTVf0WEfe9I5TMpWtzzgkJ1flh6MXzt+PY5kVdxzYpqyvQbrridjYd44/+d6/7ZNgEIXiHBgil9/y3W2FgrzC1rgMgM630NX/cxOwHtgo15sOkwSlZQA89KNknQm9gVLvRFcG1skXHQ7L4KCSrOba62fi705ucgcj6lJh72fpXNqhetiKuIKmtGK+MSX1ztYJV2ymHue9TZWNkfl6mBV5VbAhrCqSnK82W0OkVNkbv2khSNDbQM5E87AhZOj34v2f45wTp2ssZZSI/eD9K8OKZ1zqhA61tYm8apKrhZJ9hdoYxfYFM1rFhcEARB+7IrPGZKxaeOzXmatKu65/m8zeYxvDdOXptc+3TFU5NgmsJU095yK+FDD2psy5Bep6ZG+/TH/FAqWncjjO7TIbGPfmsKfmI26UNAgC1bh2/+pgQGsfdoIEoboY3LZzzCUQ1tAzf8yOxc5+Sl54Fw=";

#if defined(_WIN32) || defined(_WIN64)
static WString input_path = WString::FromLocal("../input_files/");
#else
static WString input_path = WString::FromLocal("./input_files/");
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


void OutputPDFAFixupData(ResultInformation result_info, const WString& output_txt_path) {
  TextDoc text_doc(output_txt_path, L"w+b");

  int fixup_count = result_info.GetFixupDataCount();
  char temp_string[128];
#if defined(_WIN32) || defined(_WIN64)
  sprintf_s(temp_string, 128, "== Fixup Data, count:%d ==\r\n", fixup_count);
#else
  sprintf(temp_string, "== Fixup Data, count:%d ==\r\n", fixup_count);
#endif  //defined(_WIN32) || defined(_WIN64)
  text_doc.Write(temp_string);
  for (int i = 0; i<fixup_count; i++) {
    FixupData fixup_data = result_info.GetFixupData(i);
#if defined(_WIN32) || defined(_WIN64)
    sprintf_s(temp_string, 128, "Used count:%d\r\n", fixup_data.used_count);
#else
    sprintf(temp_string, "Used count:%d\r\n", fixup_data.used_count);
#endif  // defined(_WIN32) || defined(_WIN64)
    text_doc.Write(temp_string);

    text_doc.Write("Name:");
    text_doc.Write(fixup_data.name);
    text_doc.Write("\r\n");

    text_doc.Write("Comment:");
    text_doc.Write(fixup_data.comment);
    text_doc.Write("\r\n");

    text_doc.Write("Reason:");
    int reason_count = fixup_data.reasons.GetSize();
    if (reason_count < 1) {
      text_doc.Write("\r\n");
    } else {
      for (int z = 0; z<reason_count; z++) {
        text_doc.Write("\t");
        text_doc.Write(fixup_data.reasons[z]);
        text_doc.Write("\r\n");
      }
    }

#if defined(_WIN32) || defined(_WIN64)
    sprintf_s(temp_string, 128, "State value:%d\r\n", fixup_data.state);
#else
    sprintf(temp_string, "State value:%d\r\n", fixup_data.state);
#endif  // defined(_WIN32) || defined(_WIN64)
    text_doc.Write(temp_string);

    text_doc.Write("\r\n");
  }
}

void OutputPDFAHitData(ResultInformation result_info, const WString& output_txt_path) {
  TextDoc text_doc(output_txt_path, L"w+b");

  int hit_data_count = result_info.GetHitDataCount();
  char temp_string[128];
#if defined(_WIN32) || defined(_WIN64)
  sprintf_s(temp_string, 128, "== Hit Data, count:%d ==\r\n", hit_data_count);
#else
  sprintf(temp_string, "== Hit Data, count:%d ==\r\n", hit_data_count);
#endif  // defined(_WIN32) || defined(_WIN64)
  text_doc.Write(temp_string);
  for (int i = 0; i<hit_data_count; i++) {
    HitData hit_data = result_info.GetHitData(i);
#if defined(_WIN32) || defined(_WIN64)
    sprintf_s(temp_string, 128, "Triggered count:%d\r\n", hit_data.triggered_count);
#else
    sprintf(temp_string, "Triggered count:%d\r\n", hit_data.triggered_count);
#endif  // defined(_WIN32) || defined(_WIN64)
    text_doc.Write(temp_string);

    text_doc.Write("Name:");
    text_doc.Write(hit_data.name);
    text_doc.Write("\r\n");

    text_doc.Write("Comment:");
    text_doc.Write(hit_data.comment);
    text_doc.Write("\r\n");

    text_doc.Write("Trigger value:");
    int trigger_value_count = hit_data.trigger_values.GetSize();
    if (trigger_value_count < 1) {
      text_doc.Write("\r\n");
    } else {
      for (int z = 0; z<trigger_value_count; z++) {
        text_doc.Write("\t");
        text_doc.Write(hit_data.trigger_values[z]);
        text_doc.Write("\r\n");
      }
    }

#if defined(_WIN32) || defined(_WIN64)
    sprintf_s(temp_string, 128, "Check severity:%d\r\nPage index:%d\r\n", hit_data.severity, hit_data.page_index);
#else
    sprintf(temp_string, "Check severity:%d\r\nPage index:%d\r\n", hit_data.severity, hit_data.page_index);
#endif  // defined(_WIN32) || defined(_WIN64)
    text_doc.Write(temp_string);

    text_doc.Write("\r\n");
  }
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

int main(int argc, char *argv[])
{
  CliOptions options;
  if (!ParseArgs(argc, argv, options)) {
    PrintUsage();
    return 1;
  }
  if (options.show_help) {
    PrintUsage();
    return 0;
  }
  if (!ValidateArgs(options)) {
    PrintUsage();
    return 1;
  }

  EnsureOutputDir(GetParentDir(options.output_file));

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  bool compliance_engine_initialized = false;
  try {
    WString input_file = WString::FromUTF8(options.input_file.c_str());
    WString output_file = WString::FromUTF8(options.output_file.c_str());
    WString compliance_resource_folder_path = WString::FromUTF8(options.resource_dir.c_str());
    const char* compliance_engine_unlockcode = "";

    ErrorCode compliance_error = ComplianceEngine::Initialize(compliance_resource_folder_path, compliance_engine_unlockcode);
    if (compliance_error != foxit::e_ErrSuccess) {
      if (compliance_error == foxit::e_ErrInvalidLicense) {
        printf("[Failed] Compliance module is not contained in current Foxit PDF SDK keys.\n");
      } else {
        printf("[Failed] Fail to initialize compliance engine. Error: %d\n", compliance_error);
      }
      return 1;
    }
    compliance_engine_initialized = true;

    std::string base = options.output_file + ".compliance";
    WString progress_path = WString::FromUTF8((base + ".progress.txt").c_str());
    WString fixup_path = WString::FromUTF8((base + ".fixup.txt").c_str());
    WString hit_path = WString::FromUTF8((base + ".hit.txt").c_str());

    MyComplianceProgressCallback progress_callback(progress_path);
    ResultInformation result_info;
    ComplianceType target_type = ParseComplianceType(options.type_name);

    if (target_type == e_ComplianceTypePDFA) {
      PDFACompliance compliance;
      cout << "======== PDFACompliance: Convert ========" << endl;
      result_info = compliance.ConvertPDFFile(input_file, output_file, PDFACompliance::e_VersionPDFA1a, &progress_callback);
    } else if (target_type == e_ComplianceTypePDFX) {
      PDFXCompliance compliance;
      cout << "======== PDFXCompliance: Convert ========" << endl;
      result_info = compliance.ConvertPDFFile(input_file, output_file, PDFXCompliance::e_VersionPDFX1a,
        ComplianceEngine::e_ViewOrPrintConditionAuto, false, false, &progress_callback);
    } else if (target_type == e_ComplianceTypePDFE) {
      PDFECompliance compliance;
      cout << "======== PDFECompliance: Convert ========" << endl;
      result_info = compliance.ConvertPDFFile(input_file, output_file, PDFECompliance::e_VersionPDFE1, &progress_callback);
    } else {
      printf("Error: unsupported --type value '%s'.\n", options.type_name.c_str());
      return 1;
    }

    OutputPDFAFixupData(result_info, fixup_path);
    OutputPDFAHitData(result_info, hit_path);

    ComplianceEngine::Release();
    compliance_engine_initialized = false;

    cout << "[OK] Converted with type " << ToUpper(options.type_name)
         << ", output: " << options.output_file << endl;
    cout << "[OK] Report files: " << (base + ".progress.txt") << ", "
         << (base + ".fixup.txt") << ", " << (base + ".hit.txt") << endl;

  } catch (const Exception& e) {
    if (compliance_engine_initialized) {
      ComplianceEngine::Release();
    }
    cout << e.GetMessage() << endl;
    err_ret = 1;
  }
  catch(...)
  {
    if (compliance_engine_initialized) {
      ComplianceEngine::Release();
    }
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}

