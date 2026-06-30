// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to convert DWG files to PDF files.

#if (defined(_WIN32) || defined(_WIN64) || defined (__APPLE__) || defined (__linux__)) && !defined(__arm__)

#include<string.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <fstream>
#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/addon/conversion/fs_convert.h"

using namespace std;
using namespace foxit;
using foxit::common::Library;
using namespace pdf;
using namespace::foxit::addon::conversion;

static const char* sn = "aCMBgcI1cGg8vva/PZymXOxUzeCArJTO/pNpgw6H5xbwy+GIf9iDHw==";
static const char* key = "8f3g1cGNtR8NAgfSNveBvSpWy3fmrw36K9THU1dVWbS/7f9SBAOscZwR9akWHzm7Kcrnzcm6NmTVIew7LZDgqGm2HrEBeOGXSpmIltb3Vcj10cMyK1CNug+n75fiiz20jE6wBaFQJlX5+MwKoqH0g0Tqsg4wv28Ek0GekQS7riYTpjF/eiV6gi9GLe2Ej777oWATX4OlF3JbTBNNhZxfL56cDUvhrl+m0qg9F0Ab7h35zulg2KIWTM32ejzloklLhl4X25nMHElm5hyuZeo6DJ6uCFlyfKlQQzfp0sdskNieIkXunbuF7s22ds7q6Jzk+jl2S5039+81XSM2Z7Cf9+M1asfVD7uP3FHAjuTuqrr6PF6inmhwwnGGlm3zo6/J06npCIcO0qPx0xGjBvJfkG5BR0Tu6A25CzPsM+ies5q9AdtlePkEVroRrZTid5SFS0JXPg8b3RAgyu2Tg28oDWwqYtlWXmwohaX2ZIZeoK0Innx4shV69vYJkV4jt4I5mPcrQvyjwZJcusbzMiycM3r2JlGpKyTmJ44ej5T30f7X9XVw/yN2pWCLeclftz/+edtNcK17YPPotmytbvYv0bcw2Wxa5niF8sdyUDhWYX7zj2/QLqjKS2V1kNi5bQlm4Kafg6k5FC44x4yVUEUh8gtRitn1uPAkZXQHJzFJMGWjZPcOwNdNSekhO5r/uHafD/jNTgOLFMFTw6ICUit9AFplYg0eUADSCeu3hqvaKunLXTjlAu3nE7FFB+JliE2H+PdR1/lom3wTZ/SuMfxx+lG/gpF2oaH1bg21WE59TE8mL0htTxqTo8pRr1+8nrECyWMcOsw1jgfEBUGv/ArGHN+F+HUgTlMT3GJyusoeZdcpSWEhh11UCInTxJJA2DDjQ44sGqAcYANr+gp4gPiB+ixP3FmhAeYsYc1Wb2bj96fZXm54SjPNoD6s0P6AhTx5tk1UU02Gwwsm68Ej5GgNFKfWm3e+V3H8B/xYVxDHJG3r3KAQVNl7DxVVmxDFBCd4YnxiPtluhIZE4KvtdwqxyfpVwK/KMRINPB+fppcKN/blM00DCKiSJPvKMShSS3HNUOACJCDRIjwdZrIE7cuoeZ+BUFLltbWma/uiMQgKcGtVb0acDXIznm989V2CqKuBh8pikCdbmKsfEVhHz4TKDZwk8FkltgBgWRVvSHbwf5bd52rhSxD3UgeeZClOmL+RH4MwZa40xwM6F4XpkYPl0MLzEMmlN7t8UMYRbSjDaDtgQNZQutfIHHxDLnHrC+NC7I7bbq+eF6wfMl3Ex7xbnl/frO2MC2Nuyxd3JLvMbvMdk3tvKmJmT5U31jmsu0c3uTCtC+/OOVso2gutrmrUTA==";

struct CliOptions {
  std::string input_file;
  std::string output_file;
  std::string engine_path;
  uint32 export_flags;
  DWG2PDFSettingData::DWG2PDFExportHatchesType export_hatches_type;
  DWG2PDFSettingData::DWG2PDFExportHatchesType other_export_hatches_type;
  DWG2PDFSettingData::DWG2PDFExportHatchesType gradient_export_hatches_type;
  DWG2PDFSettingData::DWG2PDFSearchableTextType searchable_text_type;
  DWG2PDFSettingData::DWG2PDFColorPolicy color_policy;
  bool is_active_layout;
  float paper_width;
  float paper_height;
  bool is_output_progress;
  std::string output_title;
  std::string output_author;
  std::string output_subject;
  std::string output_keywords;
  std::string output_creator;
  std::string output_producer;
  bool show_help;

  CliOptions()
      : export_flags(DWG2PDFSettingData::e_FlagZoomToExtentsMode)
      , export_hatches_type(DWG2PDFSettingData::e_DWG2PDFExportHatchesTypeBitmap)
      , other_export_hatches_type(DWG2PDFSettingData::e_DWG2PDFExportHatchesTypeBitmap)
      , gradient_export_hatches_type(DWG2PDFSettingData::e_DWG2PDFExportHatchesTypeBitmap)
      , searchable_text_type(DWG2PDFSettingData::e_DWG2PDFSearchableTextTypeNoSearch)
      , color_policy(DWG2PDFSettingData::e_DWG2PDFColorPolicyNoPolicy)
      , is_active_layout(true)
      , paper_width(650.0f)
      , paper_height(850.0f)
      , is_output_progress(true)
      , output_title("DWG2PDF")
      , output_author("Cad Artist")
      , output_subject("Cad Subject")
      , output_keywords("DWG2PDF")
      , output_creator("Dwg Artist")
      , output_producer("FoxitSDK")
      , show_help(false) {}
};

static void PrintUsage() {
  printf("Usage: dwg2pdf_xxx -i <input.dwg> -o <output.pdf> [options]\n"
         "\n"
         "Required:\n"
         "  -i, --input <file>                 Input DWG file path\n"
         "  -o, --output <file>                Output PDF file path (must not exist)\n"
         "      --engine <path>                Engine file path used by DWG converter\n"
         "\n"
         "Optional:\n"
         "      --export-flags <number>        DWG2PDF export flags (dec or hex, default: 0x800)\n"
         "      --solid-hatches <type>         bitmap|drawing|pdfpaths|polygons\n"
         "      --other-hatches <type>         bitmap|drawing\n"
         "      --gradient-hatches <type>      bitmap|drawing\n"
         "      --searchable-text <type>       none|shx|ttf\n"
         "      --color-policy <type>          none|mono|grayscale\n"
         "      --active-layout <true|false>   Export active layout only\n"
         "      --paper-width <mm>             Paper width in millimeters\n"
         "      --paper-height <mm>            Paper height in millimeters\n"
         "      --output-progress <true|false> Enable progress output\n"
         "      --title <text>                 PDF title metadata\n"
         "      --author <text>                PDF author metadata\n"
         "      --subject <text>               PDF subject metadata\n"
         "      --keywords <text>              PDF keywords metadata\n"
         "      --creator <text>               PDF creator metadata\n"
         "      --producer <text>              PDF producer metadata\n"
         "      --help                         Show this help message\n");
}

static bool FileExists(const std::string& path) {
  std::ifstream f(path.c_str(), std::ios::binary);
  return f.good();
}

static std::string GetParentDir(const std::string& path) {
  size_t pos = path.find_last_of("\\/");
  if (pos == std::string::npos) return "";
  return path.substr(0, pos);
}

static bool EnsureOutputDir(const std::string& dir_path) {
  if (dir_path.empty()) return true;

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
      if (_mkdir(current.c_str()) != 0 && errno != EEXIST) {
        printf("Error: failed to create output directory: %s\n", current.c_str());
        return false;
      }
#else
      if (mkdir(current.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0 && errno != EEXIST) {
        printf("Error: failed to create output directory: %s\n", current.c_str());
        return false;
      }
#endif
    }

    if (sep == std::string::npos) break;
    start = sep + 1;
  }

  return true;
}

static bool ParseBoolValue(const std::string& value, bool& out) {
  if (value == "1" || value == "true" || value == "TRUE" || value == "yes" || value == "YES") {
    out = true;
    return true;
  }
  if (value == "0" || value == "false" || value == "FALSE" || value == "no" || value == "NO") {
    out = false;
    return true;
  }
  return false;
}

static bool ParseUInt32Value(const std::string& value, uint32& out) {
  char* end = NULL;
  unsigned long result = strtoul(value.c_str(), &end, 0);
  if (end == value.c_str() || *end != '\0' || result > 0xFFFFFFFFUL) {
    return false;
  }
  out = (uint32)result;
  return true;
}

static bool ParseFloatValue(const std::string& value, float& out) {
  char* end = NULL;
  double result = strtod(value.c_str(), &end);
  if (end == value.c_str() || *end != '\0') return false;
  out = (float)result;
  return true;
}

static bool ParseHatchesValue(const std::string& value, DWG2PDFSettingData::DWG2PDFExportHatchesType& out) {
  if (value == "bitmap") {
    out = DWG2PDFSettingData::e_DWG2PDFExportHatchesTypeBitmap;
    return true;
  }
  if (value == "drawing") {
    out = DWG2PDFSettingData::e_DWG2PDFExportHatchesTypeDrawing;
    return true;
  }
  if (value == "pdfpaths") {
    out = DWG2PDFSettingData::e_DWG2PDFExportHatchesTypePdfPaths;
    return true;
  }
  if (value == "polygons") {
    out = DWG2PDFSettingData::e_DWG2PDFExportHatchesTypePolygons;
    return true;
  }
  return false;
}

static bool ParseOtherGradientHatchesValue(const std::string& value, DWG2PDFSettingData::DWG2PDFExportHatchesType& out) {
  if (value == "bitmap") {
    out = DWG2PDFSettingData::e_DWG2PDFExportHatchesTypeBitmap;
    return true;
  }
  if (value == "drawing") {
    out = DWG2PDFSettingData::e_DWG2PDFExportHatchesTypeDrawing;
    return true;
  }
  return false;
}

static bool ParseSearchableTextValue(const std::string& value, DWG2PDFSettingData::DWG2PDFSearchableTextType& out) {
  if (value == "none") {
    out = DWG2PDFSettingData::e_DWG2PDFSearchableTextTypeNoSearch;
    return true;
  }
  if (value == "shx") {
    out = DWG2PDFSettingData::e_DWG2PDFSearchableTextTypeSHX;
    return true;
  }
  if (value == "ttf") {
    out = DWG2PDFSettingData::e_DWG2PDFSearchableTextTypeTTF;
    return true;
  }
  return false;
}

static bool ParseColorPolicyValue(const std::string& value, DWG2PDFSettingData::DWG2PDFColorPolicy& out) {
  if (value == "none") {
    out = DWG2PDFSettingData::e_DWG2PDFColorPolicyNoPolicy;
    return true;
  }
  if (value == "mono") {
    out = DWG2PDFSettingData::e_DWG2PDFColorPolicyMono;
    return true;
  }
  if (value == "grayscale") {
    out = DWG2PDFSettingData::e_DWG2PDFColorPolicyGrayscale;
    return true;
  }
  return false;
}

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

static bool ParseArgs(int argc, char* argv[], CliOptions& options) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--help") {
      options.show_help = true;
      return true;
    }

    if (i + 1 >= argc) {
      printf("Missing value for '%s'.\n", argv[i]);
      return false;
    }

    std::string value = argv[++i];
    if (arg == "-i" || arg == "--input") {
      options.input_file = value;
    } else if (arg == "-o" || arg == "--output") {
      options.output_file = value;
    } else if (arg == "--engine") {
      options.engine_path = value;
    } else if (arg == "--export-flags") {
      if (!ParseUInt32Value(value, options.export_flags)) {
        printf("Invalid --export-flags value: %s\n", value.c_str());
        return false;
      }
    } else if (arg == "--solid-hatches") {
      if (!ParseHatchesValue(value, options.export_hatches_type)) {
        printf("Invalid --solid-hatches value: %s. Supported: bitmap|drawing|pdfpaths|polygons\n", value.c_str());
        return false;
      }
    } else if (arg == "--other-hatches") {
      if (!ParseOtherGradientHatchesValue(value, options.other_export_hatches_type)) {
        printf("Invalid --other-hatches value: %s. Supported: bitmap|drawing\n", value.c_str());
        return false;
      }
    } else if (arg == "--gradient-hatches") {
      if (!ParseOtherGradientHatchesValue(value, options.gradient_export_hatches_type)) {
        printf("Invalid --gradient-hatches value: %s. Supported: bitmap|drawing\n", value.c_str());
        return false;
      }
    } else if (arg == "--searchable-text") {
      if (!ParseSearchableTextValue(value, options.searchable_text_type)) {
        printf("Invalid --searchable-text value: %s. Supported: none|shx|ttf\n", value.c_str());
        return false;
      }
    } else if (arg == "--color-policy") {
      if (!ParseColorPolicyValue(value, options.color_policy)) {
        printf("Invalid --color-policy value: %s. Supported: none|mono|grayscale\n", value.c_str());
        return false;
      }
    } else if (arg == "--active-layout") {
      if (!ParseBoolValue(value, options.is_active_layout)) {
        printf("Invalid --active-layout value: %s. Supported: true|false\n", value.c_str());
        return false;
      }
    } else if (arg == "--paper-width") {
      if (!ParseFloatValue(value, options.paper_width) || options.paper_width <= 0.0f) {
        printf("Invalid --paper-width value: %s. Value must be > 0\n", value.c_str());
        return false;
      }
    } else if (arg == "--paper-height") {
      if (!ParseFloatValue(value, options.paper_height) || options.paper_height <= 0.0f) {
        printf("Invalid --paper-height value: %s. Value must be > 0\n", value.c_str());
        return false;
      }
    } else if (arg == "--output-progress") {
      if (!ParseBoolValue(value, options.is_output_progress)) {
        printf("Invalid --output-progress value: %s. Supported: true|false\n", value.c_str());
        return false;
      }
    } else if (arg == "--title") {
      options.output_title = value;
    } else if (arg == "--author") {
      options.output_author = value;
    } else if (arg == "--subject") {
      options.output_subject = value;
    } else if (arg == "--keywords") {
      options.output_keywords = value;
    } else if (arg == "--creator") {
      options.output_creator = value;
    } else if (arg == "--producer") {
      options.output_producer = value;
    } else {
      printf("Unknown argument: %s\n", arg.c_str());
      return false;
    }
  }

  return true;
}

static bool ValidateArgs(const CliOptions& options) {
  if (options.input_file.empty()) {
    printf("Error: missing required input file. Use -i or --input.\n");
    return false;
  }
  if (options.output_file.empty()) {
    printf("Error: missing required output file. Use -o or --output.\n");
    return false;
  }
  if (options.input_file == options.output_file) {
    printf("Error: input and output files must be different.\n");
    return false;
  }
  if (!FileExists(options.input_file)) {
    printf("Error: input file does not exist or cannot be opened: %s\n", options.input_file.c_str());
    return false;
  }
  if (FileExists(options.output_file)) {
    printf("Error: output file already exists (no-overwrite): %s\n", options.output_file.c_str());
    return false;
  }
  return true;
}

int main(int argc, char *argv[]) {
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
  if (!EnsureOutputDir(GetParentDir(options.output_file))) {
    return 1;
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  WString dwg_file_path = WString::FromUTF8(options.input_file.c_str());
  WString output_file_path = WString::FromUTF8(options.output_file.c_str());
  WString engine_path = WString::FromUTF8(options.engine_path.c_str());

  try {
    // Setting DWG2PDFSettingData values.
    DWG2PDFSettingData settings;
    settings.export_flags = options.export_flags;
    settings.export_hatches_type = options.export_hatches_type;
    settings.other_export_hatches_type = options.other_export_hatches_type;
    settings.gradient_export_hatches_type = options.gradient_export_hatches_type;
    settings.searchable_text_type = options.searchable_text_type;
    settings.is_active_layout = options.is_active_layout;
    settings.paper_width = options.paper_width;
    settings.paper_height = options.paper_height;
    settings.color_policy = options.color_policy;
    settings.is_output_progress = options.is_output_progress;
    settings.output_title = WString::FromUTF8(options.output_title.c_str());
    settings.output_author = WString::FromUTF8(options.output_author.c_str());
    settings.output_subject = WString::FromUTF8(options.output_subject.c_str());
    settings.output_keywords = WString::FromUTF8(options.output_keywords.c_str());
    settings.output_creator = WString::FromUTF8(options.output_creator.c_str());
    settings.output_producer = WString::FromUTF8(options.output_producer.c_str());

    // Convert DWG file to PDF file.
    Convert::FromDWG(engine_path, dwg_file_path, output_file_path, settings);
    cout << "Convert DWG to PDF successfully: " << options.output_file << endl;
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  } catch (...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }
  return err_ret;
}
#endif

