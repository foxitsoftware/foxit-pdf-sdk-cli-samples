// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to export
// metadata information and viewer preference information from a PDF file.

// Include Foxit SDK header files.
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>
#include <cerrno>
#include <sys/stat.h>
#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/fs_pdfdocviewerprefs.h"
#include "../../../include/pdf/fs_pdfmetadata.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace common;
using namespace graphics;
using namespace objects;

enum OutputMode {
  kModeMetadata = 0,
  kModeView = 1,
  kModeBoth = 2
};

struct CliOptions {
  std::string input_file;
  std::string output_file;
  OutputMode mode;
  bool show_help;

  CliOptions() : mode(kModeMetadata), show_help(false) {}
};

static std::string ToUpperString(const std::string& value) {
  std::string out = value;
  for (size_t i = 0; i < out.size(); ++i) {
    if (out[i] >= 'a' && out[i] <= 'z') out[i] = (char)(out[i] - ('a' - 'A'));
  }
  return out;
}

static const char* ModeToString(OutputMode mode) {
  switch (mode) {
    case kModeMetadata: return "metadata";
    case kModeView: return "view";
    case kModeBoth: return "both";
    default: return "metadata";
  }
}

static bool ParseMode(const std::string& value, OutputMode& mode) {
  std::string normalized = ToUpperString(value);
  if (normalized == "METADATA") {
    mode = kModeMetadata;
    return true;
  }
  if (normalized == "VIEW") {
    mode = kModeView;
    return true;
  }
  if (normalized == "BOTH") {
    mode = kModeBoth;
    return true;
  }
  return false;
}

static void PrintUsage() {
  cout
    << "Usage: docinfo -i <input.pdf> -o <output.txt> [--mode <metadata|view|both>]" << endl
    << endl
    << "Required options:" << endl
    << "  -i <file>                Input PDF file path" << endl
    << "  -o <file>                Output metadata report path" << endl
    << endl
    << "Optional options:" << endl
    << "  --mode <type>            Output mode: metadata (default), view, both" << endl
    << "  --help                   Show this help message" << endl;
}

static bool ParseArgs(int argc, char* argv[], CliOptions& options) {
  for (int i = 1; i < argc; ++i) {
    String arg(argv[i]);
    if (arg.Equal("--help")) {
      options.show_help = true;
      return true;
    }
    if (i + 1 >= argc) {
      printf("Missing value for argument: %s\n", argv[i]);
      return false;
    }

    if (arg.Equal("-i")) {
      options.input_file = argv[++i];
    } else if (arg.Equal("-o")) {
      options.output_file = argv[++i];
    } else if (arg.Equal("--mode")) {
      std::string mode_value = argv[++i];
      if (!ParseMode(mode_value, options.mode)) {
        printf("Invalid mode: %s. Supported values: metadata, view, both.\n", mode_value.c_str());
        return false;
      }
    } else {
      printf("Unknown argument: %s\n", argv[i]);
      return false;
    }
  }
  return true;
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

static bool ValidateArgs(const CliOptions& options) {
  if (options.input_file.empty()) {
    printf("Error: -i is required.\n");
    return false;
  }
  if (options.output_file.empty()) {
    printf("Error: -o is required.\n");
    return false;
  }
  if (options.input_file == options.output_file) {
    printf("Error: input and output file paths must be different.\n");
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

static std::string ToLocalString(const WString& text) {
  return std::string((const char*)String::FromUnicode(text));
}

static std::string DateTimeToString(const DateTime& date_time) {
  if (date_time.year == 0 && date_time.month == 0 && date_time.day == 0 &&
      date_time.hour == 0 && date_time.minute == 0 && date_time.second == 0) {
    return "(empty)";
  }

  char buffer[128];
#if defined(_WIN32) || defined(_WIN64)
  sprintf_s(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d.%03d UTC%+03d:%02d",
    date_time.year, date_time.month, date_time.day,
    date_time.hour, date_time.minute, date_time.second,
    date_time.milliseconds, date_time.utc_hour_offset, date_time.utc_minute_offset);
#else
  sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d.%03d UTC%+03d:%02d",
    date_time.year, date_time.month, date_time.day,
    date_time.hour, date_time.minute, date_time.second,
    date_time.milliseconds, date_time.utc_hour_offset, date_time.utc_minute_offset);
#endif
  return std::string(buffer);
}

static const char* BoolToText(bool value) {
  return value ? "true" : "false";
}

static const char* DisplayModeToText(PDFDoc::DisplayMode mode) {
  switch (mode) {
    case PDFDoc::e_DisplayUseNone: return "UseNone";
    case PDFDoc::e_DisplayUseOutlines: return "UseOutlines";
    case PDFDoc::e_DisplayUseThumbs: return "UseThumbs";
    case PDFDoc::e_DisplayUseOC: return "UseOC";
    default: return "Unknown";
  }
}

static const char* BoxTypeToText(PDFPage::BoxType box_type) {
  switch (box_type) {
    case PDFPage::e_MediaBox: return "MediaBox";
    case PDFPage::e_CropBox: return "CropBox";
    case PDFPage::e_BleedBox: return "BleedBox";
    case PDFPage::e_TrimBox: return "TrimBox";
    case PDFPage::e_ArtBox: return "ArtBox";
    default: return "Unknown";
  }
}

static const char* PrintScaleToText(DocViewerPrefs::PrintScale print_scale) {
  switch (print_scale) {
    case DocViewerPrefs::e_PrintScaleNone: return "None";
    case DocViewerPrefs::e_PrintScaleAppDefault: return "AppDefault";
    default: return "Unknown";
  }
}

static void WriteMetadataValues(std::ostream& output, Metadata& metadata, const wchar_t* key, const char* label) {
  output << label << ": ";
  WStringArray values = metadata.GetValues(key);
  if (values.GetSize() < 1) {
    output << "(empty)" << std::endl;
    return;
  }

  for (size_t i = 0; i < values.GetSize(); ++i) {
    if (i > 0) output << "; ";
    output << ToLocalString(values[i]);
  }
  output << std::endl;
}

static bool WriteReportToFile(const WString& output_file, const std::string& report) {
#if defined(_WIN32) || defined(_WIN64)
  FILE* output = _wfopen((const wchar_t*)output_file, L"wb");
#else
  FILE* output = fopen((const char*)String::FromUnicode(output_file), "wb");
#endif
  if (output == NULL) {
    printf("[Failed] Cannot open output file: %s\n", (const char*)String::FromUnicode(output_file));
    return false;
  }

  size_t written = fwrite(report.data(), 1, report.size(), output);
  fclose(output);
  if (written != report.size()) {
    printf("[Failed] Cannot write metadata report: %s\n", (const char*)String::FromUnicode(output_file));
    return false;
  }
  return true;
}

static void AppendMetadataReport(std::ostream& output, PDFDoc& doc) {
  Metadata metadata(doc);
  output << "Metadata Report" << std::endl;
  output << "===============" << std::endl;
  WriteMetadataValues(output, metadata, L"Title", "Title");
  WriteMetadataValues(output, metadata, L"Author", "Author");
  WriteMetadataValues(output, metadata, L"Subject", "Subject");
  WriteMetadataValues(output, metadata, L"Keywords", "Keywords");
  WriteMetadataValues(output, metadata, L"Creator", "Creator");
  WriteMetadataValues(output, metadata, L"Producer", "Producer");
  WriteMetadataValues(output, metadata, L"Trapped", "Trapped");
  WriteMetadataValues(output, metadata, L"pdfaid", "PDFA ID");
  WriteMetadataValues(output, metadata, L"InstanceID", "Instance ID");
  WriteMetadataValues(output, metadata, L"DocumentID", "Document ID");
  output << "CreationDate: " << DateTimeToString(metadata.GetCreationDateTime()) << std::endl;
  output << "ModDate: " << DateTimeToString(metadata.GetModifiedDateTime()) << std::endl;

  WStringArray customer_keys = metadata.GetCustomerKeys();
  output << "CustomerKeys: ";
  if (customer_keys.GetSize() < 1) {
    output << "(empty)" << std::endl;
  } else {
    output << std::endl;
    for (size_t i = 0; i < customer_keys.GetSize(); ++i) {
      std::string key_name = ToLocalString(customer_keys[i]);
      output << "  " << key_name << ": ";
      WStringArray values = metadata.GetValues(customer_keys[i]);
      if (values.GetSize() < 1) {
        output << "(empty)";
      } else {
        for (size_t z = 0; z < values.GetSize(); ++z) {
          if (z > 0) output << "; ";
          output << ToLocalString(values[z]);
        }
      }
      output << std::endl;
    }
  }
}

static void AppendViewerPreferenceReport(std::ostream& output, PDFDoc& doc) {
  DocViewerPrefs prefs(doc);
  output << "Viewer Preference Report" << std::endl;
  output << "========================" << std::endl;
  output << "HideToolbar: " << BoolToText(prefs.GetUIDisplayStatus(DocViewerPrefs::e_HideToolbar)) << std::endl;
  output << "HideMenubar: " << BoolToText(prefs.GetUIDisplayStatus(DocViewerPrefs::e_HideMenubar)) << std::endl;
  output << "HideWindowUI: " << BoolToText(prefs.GetUIDisplayStatus(DocViewerPrefs::e_HideWindowUI)) << std::endl;
  output << "FitWindow: " << BoolToText(prefs.GetUIDisplayStatus(DocViewerPrefs::e_FitWindow)) << std::endl;
  output << "CenterWindow: " << BoolToText(prefs.GetUIDisplayStatus(DocViewerPrefs::e_CenterWindow)) << std::endl;
  output << "DisplayDocTitle: " << BoolToText(prefs.GetUIDisplayStatus(DocViewerPrefs::e_DisplayDocTitle)) << std::endl;
  output << "NonFullScreenPageMode: " << DisplayModeToText(prefs.GetNonFullScreenPageMode()) << std::endl;
  output << "ReadingDirection: " << (prefs.GetReadingDirection() ? "LeftToRight" : "RightToLeft") << std::endl;
  output << "ViewArea: " << BoxTypeToText(prefs.GetViewArea()) << std::endl;
  output << "ViewClip: " << BoxTypeToText(prefs.GetViewClip()) << std::endl;
  output << "PrintArea: " << BoxTypeToText(prefs.GetPrintArea()) << std::endl;
  output << "PrintClip: " << BoxTypeToText(prefs.GetPrintClip()) << std::endl;
  output << "PrintScale: " << PrintScaleToText(prefs.GetPrintScale()) << std::endl;
  output << "PrintCopies: " << prefs.GetPrintCopies() << std::endl;
}

static bool ExportDocInfoReport(PDFDoc& doc, const WString& output_file, OutputMode mode) {
  std::ostringstream output;
  output << "DocInfo Report" << std::endl;
  output << "=============" << std::endl;
  output << "Mode: " << ModeToString(mode) << std::endl;
  output << std::endl;

  if (mode == kModeMetadata || mode == kModeBoth) {
    AppendMetadataReport(output, doc);
    if (mode == kModeBoth) output << std::endl;
  }
  if (mode == kModeView || mode == kModeBoth) {
    AppendViewerPreferenceReport(output, doc);
  }

  return WriteReportToFile(output_file, output.str());
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

  try {
    WString input_file = WString::FromUTF8(options.input_file.c_str());
    WString output_file = WString::FromUTF8(options.output_file.c_str());

    PDFDoc doc(input_file);
    ErrorCode code = doc.Load();
    if (code != foxit::e_ErrSuccess) {
      printf("[Failed] Cannot load PDF document %s.\r\nError Message: %d\r\n", (const char*)String::FromUnicode(input_file), code);
      return 1;
    }
    if (!ExportDocInfoReport(doc, output_file, options.mode)) {
      return 1;
    }
    cout << "[OK] " << ModeToString(options.mode) << " report written to: " << options.output_file << endl;

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
