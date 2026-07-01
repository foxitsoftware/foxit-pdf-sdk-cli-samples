// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to do output preview.


#include <set>
#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

// Include Foxit SDK header files.
#include "../../../include/common/fs_image.h"
#include "../../../include/common/fs_render.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/fs_outputpreview.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

struct OutputPreviewCommand {
  WString input_file;
  WString output_file;
  WString icc_file;
  int show_type;
  bool simulate_overprint;
  String spot_plates_raw;
  String process_plates_raw;

  bool has_input;
  bool has_output;
  bool has_icc;

  OutputPreviewCommand()
      : show_type(static_cast<int>(OutputPreview::e_ShowAll)),
        simulate_overprint(true),
        has_input(false),
        has_output(false),
        has_icc(false) {}
};

void PrintUsage() {
  cout << "Usage:" << endl
      << "output_preview --input <pdf path> --output <bmp path> --icc <icc path> [options]" << endl
      << endl
      << "Required:" << endl
      << "--input <path>                     Input PDF path." << endl
      << "--output <path>                    Output preview bitmap path (.bmp)." << endl
      << "--icc <path>                       Simulation ICC profile path." << endl
      << endl
      << "Optional:" << endl
      << "--show-type <0-23>                 OutputPreview::ShowType enum value." << endl
      << "--simulate-overprint <true|false>  Enable or disable overprint simulation." << endl
      << "--spot-plates <names>              Comma-separated spot plate names to enable (default: all)." << endl
      << "--process-plates <names>           Comma-separated process/separation plate names to enable (default: all)." << endl;
}

bool ParseIntValue(const String& value, int& out_value) {
  char* end_ptr = NULL;
  out_value = static_cast<int>(strtol((const char*)value, &end_ptr, 10));
  return end_ptr != NULL && *end_ptr == '\0';
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

bool IsValidShowType(int show_type) {
  return show_type >= static_cast<int>(OutputPreview::e_ShowAll)
      && show_type <= static_cast<int>(OutputPreview::e_ShowLineArt);
}

bool FileExists(const WString& path) {
  FILE* file = NULL;
#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file, String::FromUnicode(path), "rb");
#else
  file = fopen(String::FromUnicode(path), "rb");
#endif
  if (file == NULL) {
    return false;
  }
  fclose(file);
  return true;
}

WString ParentDirectory(const WString& path) {
  std::wstring wpath = std::wstring((FX_LPCWSTR)path, path.GetLength());
  size_t pos = wpath.find_last_of(L'/');
  size_t pos2 = wpath.find_last_of(L'\\');
  if (pos == (size_t)-1 || (pos2 != (size_t)-1 && pos2 > pos)) pos = pos2;
  if (pos == (size_t)-1) return L"";
  std::wstring wpathsub = wpath.substr(0, pos + 1);
  return foxit::WString(wpathsub.c_str(), wpathsub.length());
}

void EnsureDirectoryExists(const WString& directory) {
  if (directory.IsEmpty()) {
    return;
  }
#if defined(_WIN32) || defined(_WIN64)
  _mkdir(String::FromUnicode(directory));
#else
  mkdir(String::FromUnicode(directory), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

bool AnalysisParameter(int argc, char* argv[], OutputPreviewCommand& command) {
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
    } else if (key.Equal("--icc")) {
      command.icc_file = WString::FromUTF8(value);
      command.has_icc = true;
    } else if (key.Equal("--show-type")) {
      if (!ParseIntValue(value, command.show_type) || !IsValidShowType(command.show_type)) {
        return false;
      }
    } else if (key.Equal("--simulate-overprint")) {
      if (!ParseBoolValue(value, command.simulate_overprint)) {
        return false;
      }
    } else if (key.Equal("--spot-plates")) {
      command.spot_plates_raw = value;
    } else if (key.Equal("--process-plates")) {
      command.process_plates_raw = value;
    } else {
      return false;
    }
  }

  return command.has_input && command.has_output && command.has_icc;
}

// sn and key information from Foxit PDF SDK's key files are used to initialize Foxit PDF SDK library. 
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

int main(int argc, char *argv[]) {
  if ((argc > 1 && String(argv[1]).Equal("--help")) || argc < 2) {
    PrintUsage();
    return 0;
  }

  OutputPreviewCommand command;
  if (!AnalysisParameter(argc, argv, command)) {
    PrintUsage();
    return 1;
  }

  if (command.input_file == command.output_file) {
    cout << "Input and output path must be different." << endl;
    return 1;
  }

  if (!FileExists(command.input_file)) {
    cout << "Input PDF does not exist." << endl;
    return 1;
  }

  if (!FileExists(command.icc_file)) {
    cout << "ICC profile file does not exist." << endl;
    return 1;
  }

  EnsureDirectoryExists(ParentDirectory(command.output_file));

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library.
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  printf("Input file path: %s\r\n", (const char*)String::FromUnicode(command.input_file));

  WString default_icc_folder_path = ParentDirectory(command.icc_file);
  if (default_icc_folder_path.IsEmpty()) {
    std::cout << "Cannot resolve ICC folder from --icc path." << std::endl;
    return 1;
  }

  try {
    Library::SetDefaultICCProfilesPath(default_icc_folder_path);
  } catch (Exception e) {
    if (foxit::e_ErrFilePathNotExist == e.GetErrCode()) {
      std::cout << "ICC folder path does not exist." << std::endl;
      return 1;
    } else {
      cout << e.GetMessage() << endl;
      return 1;
    }
  }

  try {
    PDFDoc pdf_doc(command.input_file);
    pdf_doc.StartLoad();
    PDFPage pdf_page = pdf_doc.GetPage(0);
    pdf_page.StartParse(PDFPage::e_ParsePageNormal);
    float page_width = pdf_page.GetWidth();
    float page_height = pdf_page.GetHeight();
    int bitmap_width = (int)page_width;
    int bitmap_height = (int)page_height;
    Bitmap::DIBFormat bitmap_format = Bitmap::e_DIBInvalid;
    uint32 background_color = 0x000000;
    if (pdf_page.HasTransparency()) {
      background_color = 0x000000;
      bitmap_format = Bitmap::e_DIBArgb;
    }
    else {
      background_color = 0xFFFFFF;
      bitmap_format = Bitmap::e_DIBRgb32;
    }
    Matrix display_matrix = pdf_page.GetDisplayMatrix(0, 0, bitmap_width, bitmap_height, common::e_Rotation0);

    Bitmap render_bitmap(bitmap_width, bitmap_height, bitmap_format);
    render_bitmap.FillRect(background_color);
    Renderer renderer(render_bitmap, false);

    OutputPreview output_preview(pdf_doc);
    output_preview.SetSimulationProfile(command.icc_file);
    output_preview.SetShowType(static_cast<OutputPreview::ShowType>(command.show_type));
    output_preview.EnableSimulateOverprint(command.simulate_overprint);

    // Helper: parse comma-separated names into a set
    auto ParseNameSet = [](const String& raw) -> std::set<std::string> {
      std::set<std::string> result;
      if (raw.IsEmpty()) return result;
      std::string s((const char*)raw);
      size_t start = 0;
      while (start <= s.size()) {
        size_t comma = s.find(',', start);
        std::string token = (comma == std::string::npos) ? s.substr(start) : s.substr(start, comma - start);
        size_t b = token.find_first_not_of(" \t");
        size_t e = token.find_last_not_of(" \t");
        if (b != std::string::npos) result.insert(token.substr(b, e - b + 1));
        if (comma == std::string::npos) break;
        start = comma + 1;
      }
      return result;
    };

    std::set<std::string> spot_filter = ParseNameSet(command.spot_plates_raw);
    std::set<std::string> process_filter = ParseNameSet(command.process_plates_raw);

    // Set spot (专色) plate check status
    StringArray spot_plates = output_preview.GetPlates(OutputPreview::e_ColorantTypeSpot);
    for (int i = 0; i < (int)spot_plates.GetSize(); i++) {
      std::string name((const char*)spot_plates[i]);
      bool checked = spot_filter.empty() || spot_filter.count(name) > 0;
      output_preview.SetCheckStatus(spot_plates[i], checked);
    }

    // Set process/separation (分色) plate check status
    StringArray process_plates = output_preview.GetPlates(OutputPreview::e_ColorantTypeProcess);
    for (int i = 0; i < (int)process_plates.GetSize(); i++) {
      std::string name((const char*)process_plates[i]);
      bool checked = process_filter.empty() || process_filter.count(name) > 0;
      output_preview.SetCheckStatus(process_plates[i], checked);
    }

    Bitmap preview_bitmap = output_preview.GeneratePreviewBitmap(pdf_page, display_matrix, renderer);
    Image result_image;
    result_image.AddFrame(preview_bitmap);
    result_image.SaveAs(command.output_file);

    cout << "[END] demo output_preview." << endl;
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  } catch(...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}

