// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to print PDF pages.

#if !defined(__APPLE__)

#include <iostream>
#include <string>

#include <time.h>
#include <map>
#include <set>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/graphics/fs_pdfgraphicsobject.h"
#include "../../../include/pdf/fs_pdfpage.h"

#include "../../../include/common/fs_render.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "";
static const char* key = "";

struct PdfPrintCommand {
  WString input_file;
  WString output_dir;      // if set, render to BMP images instead of printing
  WString printer_name;    // specific printer name (empty = default)
  int start_page;          // 1-based, default 1
  int end_page;            // 1-based, default last page
  int rotation;            // 0, 90, 180, 270
  bool show_help;

  PdfPrintCommand()
      : start_page(1),
        end_page(0),
        rotation(0),
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
  cout << "pdfprint --input <input.pdf> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --input <path>                  Input PDF file path." << endl << endl;
  cout << "Output mode (choose one):" << endl;
  cout << "  (default)                       Print to system default printer." << endl;
  cout << "  --printer <name>                Print to specified printer." << endl;
  cout << "  --output <dir>                  Render pages to BMP images in directory." << endl << endl;
  cout << "Page range:" << endl;
  cout << "  --start-page <int>              Start page (1-based). Default: 1." << endl;
  cout << "  --end-page <int>                End page (1-based). Default: last page." << endl << endl;
  cout << "Render options:" << endl;
  cout << "  --rotation <0|90|180|270>       Page rotation. Default: 0." << endl << endl;
  cout << "Optional:" << endl;
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

Rotation ParseRotation(int value) {
  switch (value) {
    case 90:  return e_Rotation90;
    case 180: return e_Rotation180;
    case 270: return e_Rotation270;
    default:  return e_Rotation0;
  }
}

bool ParseCommand(int argc, char* argv[], PdfPrintCommand& command) {
  if (argc <= 1) {
    PrintUsage();
    return false;
  }

  bool has_input = false;
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
      command.output_dir = WString::FromUTF8(value);
    } else if (key.Equal("--printer") || key.Equal("-p")) {
      command.printer_name = WString::FromUTF8(value);
    } else if (key.Equal("--start-page")) {
      command.start_page = atoi((const char*)value);
    } else if (key.Equal("--end-page")) {
      command.end_page = atoi((const char*)value);
    } else if (key.Equal("--rotation") || key.Equal("-r")) {
      int rot = atoi((const char*)value);
      if (rot != 0 && rot != 90 && rot != 180 && rot != 270) {
        printf("Invalid rotation: %d (must be 0, 90, 180, or 270)\n", rot);
        return false;
      }
      command.rotation = rot;
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (!has_input) {
    printf("--input is required.\n");
    PrintUsage();
    return false;
  }
  if (!FileExists(command.input_file)) {
    printf("Input file does not exist: %s\n", (const char*)String::FromUnicode(command.input_file));
    return false;
  }
  return true;
}

int main(int argc, char* argv[]) {
  PdfPrintCommand command;
  if (!ParseCommand(argc, argv, command)) return 1;

  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) return 1;

  try {
    PDFDoc doc(command.input_file);
    ErrorCode code = doc.Load();
    if (code != foxit::e_ErrSuccess) {
      printf("Error: Load PDF \"%s\" failed. Error code: %d\n",
             (const char*)String::FromUnicode(command.input_file), code);
      return 1;
    }

    int page_count = doc.GetPageCount();
    int start_page = (command.start_page >= 1) ? command.start_page - 1 : 0;
    int end_page = (command.end_page > 0) ? command.end_page : page_count;
    if (start_page >= page_count) {
      printf("Error: Start page %d out of range (1-%d).\n", command.start_page, page_count);
      return 1;
    }
    if (end_page > page_count) end_page = page_count;

    Rotation rotation = ParseRotation(command.rotation);

    // Mode: render to BMP images
    if (!command.output_dir.IsEmpty()) {
#if defined(_WIN32) || defined(_WIN64)
      _mkdir(String::FromUnicode(command.output_dir));
#else
      mkdir(String::FromUnicode(command.output_dir), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
      for (int i = start_page; i < end_page; i++) {
        PDFPage page = doc.GetPage(i);
        page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);

        int width = static_cast<int>(page.GetWidth());
        int height = static_cast<int>(page.GetHeight());
        Matrix matrix = page.GetDisplayMatrix(0, 0, width, height, rotation);

        Bitmap bitmap(width, height, foxit::common::Bitmap::e_DIBArgb, NULL, 0);
        bitmap.FillRect(0xFFFFFFFF, NULL);

        Renderer render(bitmap, false);
        render.StartRender(page, matrix, NULL);

        Image image;
        image.AddFrame(bitmap);

        WString file_name;
        file_name.Format((FX_LPCWSTR)L"%spage_%d.bmp", (const wchar_t*)command.output_dir, i + 1);
        image.SaveAs(file_name);
        cout << "Rendered page " << (i + 1) << " to: " << (const char*)String::FromUnicode(file_name) << endl;
      }
      cout << "PDF rendered to BMP. Output directory: " << (const char*)String::FromUnicode(command.output_dir) << endl;
      return 0;
    }

    // Mode: print to printer (Windows only)
#if !defined(_WIN32) && !defined(_WIN64)
    printf("Error: Printing to printer is only supported on Windows.\n");
    printf("Use --output <dir> to render pages to BMP images instead.\n");
    return 1;
#else
    HDC hdc_print = NULL;
    char dev_string[120];
    char *printer = NULL, *driver = NULL;
    char *port = NULL;
    char* context = NULL;

    if (!command.printer_name.IsEmpty()) {
      // Use specified printer
      String printer_str = String::FromUnicode(command.printer_name);
      hdc_print = CreateDCA(NULL, printer_str, NULL, NULL);
    } else {
      // Use default printer
      GetProfileStringA("windows", "device", ",,,", dev_string, 120);
      if ((printer = strtok_s(dev_string, (const char *)",", &context)) &&
          (driver = strtok_s((char *)NULL, (const char *)", ", &context)) &&
          (port = strtok_s((char *)NULL, (const char *)", ", &context))) {
        hdc_print = CreateDCA(driver, printer, port, NULL);
      }
    }

    if (!hdc_print) {
      printf("Error: Cannot create HDC for print.\n");
      return 1;
    }

    char print_doc_name[128] = "";
    sprintf_s(print_doc_name, "FoxitPDFSDK_Print_%s", (const char*)String::FromUnicode(command.input_file));
    DOCINFOA doc_info;
    ZeroMemory(&doc_info, sizeof(DOCINFOA));
    doc_info.cbSize = sizeof(DOCINFOA);
    doc_info.lpszDocName = print_doc_name;
    doc_info.lpszOutput = NULL;
    doc_info.fwType = 0;

    int nError = StartDocA(hdc_print, &doc_info);
    if (nError == SP_ERROR) {
      printf("Error: StartDoc failed.\n");
      DeleteDC(hdc_print);
      return 1;
    }

    for (int i = start_page; i < end_page; i++) {
      PDFPage page = doc.GetPage(i);
      page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);

      int width = static_cast<int>(page.GetWidth());
      int height = static_cast<int>(page.GetHeight());

      nError = StartPage(hdc_print);
      if (nError == SP_ERROR) {
        printf("Error: StartPage failed for page %d.\n", i + 1);
        EndDoc(hdc_print);
        DeleteDC(hdc_print);
        return 1;
      }

      float device_width = (float)GetDeviceCaps(hdc_print, HORZRES);
      float device_height = (float)GetDeviceCaps(hdc_print, VERTRES);
      float x_scale = device_width / width;
      float y_scale = device_height / height;

      float scale = x_scale < y_scale ? x_scale : y_scale;
      float x_offset = 0.0;
      float y_offset = 0.0;
      if (x_scale < y_scale) {
        y_offset = (device_height - scale * height) / 2;
        device_height = scale * height;
      } else {
        x_offset = (device_width - scale * width) / 2;
        device_width = scale * width;
      }

      RECT rect = {(LONG)x_offset, (LONG)y_offset, (LONG)(device_width + x_offset), (LONG)(device_height + y_offset)};
      Matrix matrix = page.GetDisplayMatrix(rect.left, rect.top, (int)device_width, (int)device_height, rotation);

      HBRUSH hBrush = CreateSolidBrush(RGB(0xff, 0xff, 0xff));
      FillRect(hdc_print, &rect, hBrush);
      DeleteObject(hBrush);

      Renderer render_print(hdc_print, WString::FromLocal(dev_string));
      render_print.EnableForPrint(true);
      RectI clip_rect(rect.left, rect.top, rect.right, rect.bottom);
      render_print.SetClipRect(&clip_rect);
      render_print.StartRender(page, matrix, NULL);

      EndPage(hdc_print);
    }

    EndDoc(hdc_print);
    DeleteDC(hdc_print);
    cout << "PDF printed. Pages: " << (end_page - start_page) << endl;
#endif

  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    return 1;
  }
  catch(...) {
    cout << "Unknown Exception" << endl;
    return 1;
  }

  return 0;
}
#endif  // #if !defined(__APPLE__)

