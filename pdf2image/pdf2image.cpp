// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file is a demo demonstrate how to convert a PDF file to one or multiple image files.

#include <time.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <algorithm>
#include <cstdlib>
#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "";
static const char* key = "";

#if defined(_WIN32) || defined(_WIN64)
static WString output_path = WString::FromLocal("../output_files/");
static WString input_path = WString::FromLocal("../input_files/");
#else
static WString output_path = WString::FromLocal("./output_files/");
static WString input_path = WString::FromLocal("./input_files/");
#endif

static WString g_output_directory;
static WString g_output_prefix;

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
    if (is_initialize_) Library::Release();
  }

 private:
  bool is_initialize_;
};

struct Pdf2ImageCommand {
  WString input_file;
  WString output_directory;
  WString output_prefix;
  String pages_expr;
  String formats_expr;
  bool from_memory;
  bool enable_hdc_render;
  bool show_help;

  Pdf2ImageCommand()
      : input_file(input_path + L"AboutFoxit.pdf"),
        output_directory(output_path + L"pdf2image/"),
        output_prefix(L"AboutFoxit"),
        pages_expr("all"),
        formats_expr("all"),
        from_memory(false),
        enable_hdc_render(false),
        show_help(false) {}
};

void PrintUsage() {
  cout << "Usage:" << endl;
  cout << "pdf2image --input <input.pdf> --output-dir <directory> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --input <path>              Input PDF path." << endl;
  cout << "  --output-dir <path>         Output directory." << endl << endl;
  cout << "Optional:" << endl;
  cout << "  --pages <all|list>          Page selector, e.g. all or 1,3-5." << endl;
  cout << "  --formats <list>            Comma list: bmp,jpg,jpeg,png,jpx,jp2,tif,tiff,all." << endl;
  cout << "  --output-prefix <text>      Output file prefix (default: AboutFoxit)." << endl;
  cout << "  --from-memory <bool>        Load document from memory path (true/false)." << endl;
  cout << "  --enable-hdc-render <bool>  Windows only, export render_by_hdc_pageN.bmp." << endl;
  cout << "  --help                      Show this message." << endl;
}

bool ParseBoolValue(const String& value, bool& out_value) {
  std::string text = (const char*)value;
  std::transform(text.begin(), text.end(), text.begin(), ::tolower);
  if (text == "1" || text == "true" || text == "yes") {
    out_value = true;
    return true;
  }
  if (text == "0" || text == "false" || text == "no") {
    out_value = false;
    return true;
  }
  return false;
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

WString NormalizeOutputDir(const WString& output_dir) {
  std::string raw = (const char*)String::FromUnicode(output_dir);
  if (!raw.empty() && raw[raw.size() - 1] != '/' && raw[raw.size() - 1] != '\\') {
#if defined(_WIN32) || defined(_WIN64)
    raw += "\\";
#else
    raw += "/";
#endif
  }
  return WString::FromUTF8(raw.c_str());
}

void EnsureOutputDirectory(const WString& output_dir) {
#if defined(_WIN32) || defined(_WIN64)
  _mkdir(String::FromUnicode(output_dir));
#else
  mkdir(String::FromUnicode(output_dir), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

bool ParseCommand(int argc, char* argv[], Pdf2ImageCommand& command) {
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
    } else if (key.Equal("--output-dir") || key.Equal("-o")) {
      command.output_directory = WString::FromUTF8(value);
      has_output = true;
    } else if (key.Equal("--pages")) {
      command.pages_expr = value;
    } else if (key.Equal("--formats")) {
      command.formats_expr = value;
    } else if (key.Equal("--output-prefix")) {
      command.output_prefix = WString::FromUTF8(value);
    } else if (key.Equal("--from-memory")) {
      if (!ParseBoolValue(value, command.from_memory)) {
        printf("Invalid bool value for --from-memory: %s\n", (const char*)value);
        return false;
      }
    } else if (key.Equal("--enable-hdc-render")) {
      if (!ParseBoolValue(value, command.enable_hdc_render)) {
        printf("Invalid bool value for --enable-hdc-render: %s\n", (const char*)value);
        return false;
      }
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (!has_input || !has_output) {
    printf("Both --input and --output-dir are required.\n");
    PrintUsage();
    return false;
  }

  if (!FileExists(command.input_file)) {
    printf("Input file does not exist: %s\n", (const char*)String::FromUnicode(command.input_file));
    return false;
  }

#if !defined(_WIN32) && !defined(_WIN64)
  if (command.enable_hdc_render) {
    printf("--enable-hdc-render is only supported on Windows.\n");
    return false;
  }
#endif

  command.output_directory = NormalizeOutputDir(command.output_directory);
  return true;
}

bool ResolveFormats(const String& formats_expr,
                    std::vector<std::string>& single_formats,
                    std::vector<std::string>& multi_formats) {
  std::set<std::string> single_set;
  std::set<std::string> multi_set;
  std::string raw = (const char*)formats_expr;
  std::transform(raw.begin(), raw.end(), raw.begin(), ::tolower);

  if (raw == "all") {
    single_set.insert(".bmp");
    single_set.insert(".jpg");
    single_set.insert(".jpeg");
    single_set.insert(".png");
    single_set.insert(".jpx");
    single_set.insert(".jp2");
    multi_set.insert(".tif");
    multi_set.insert(".tiff");
  } else {
    std::stringstream ss(raw);
    std::string token;
    while (std::getline(ss, token, ',')) {
      if (token.empty()) continue;
      if (token == "bmp" || token == "jpg" || token == "jpeg" || token == "png" || token == "jpx" || token == "jp2") {
        single_set.insert("." + token);
      } else if (token == "tif" || token == "tiff") {
        multi_set.insert("." + token);
      } else {
        printf("Unsupported format token: %s\n", token.c_str());
        return false;
      }
    }
  }

  if (single_set.empty() && multi_set.empty()) {
    printf("No valid output formats resolved from --formats.\n");
    return false;
  }

  single_formats.assign(single_set.begin(), single_set.end());
  multi_formats.assign(multi_set.begin(), multi_set.end());
  return true;
}

bool ResolvePages(const String& pages_expr, int page_count, std::vector<int>& page_indices) {
  if (page_count <= 0) {
    printf("Input document has no pages.\n");
    return false;
  }

  std::set<int> page_set;
  std::string raw = (const char*)pages_expr;
  std::transform(raw.begin(), raw.end(), raw.begin(), ::tolower);
  if (raw == "all") {
    for (int i = 0; i < page_count; ++i) page_set.insert(i);
  } else {
    std::stringstream ss(raw);
    std::string token;
    while (std::getline(ss, token, ',')) {
      if (token.empty()) continue;
      size_t dash_pos = token.find('-');
      if (dash_pos == std::string::npos) {
        int page_no = atoi(token.c_str());
        if (page_no < 1 || page_no > page_count) {
          printf("Page index out of range: %d\n", page_no);
          return false;
        }
        page_set.insert(page_no - 1);
      } else {
        int start_page = atoi(token.substr(0, dash_pos).c_str());
        int end_page = atoi(token.substr(dash_pos + 1).c_str());
        if (start_page < 1 || end_page < 1 || start_page > end_page || end_page > page_count) {
          printf("Invalid page range token: %s\n", token.c_str());
          return false;
        }
        for (int p = start_page; p <= end_page; ++p) page_set.insert(p - 1);
      }
    }
  }

  if (page_set.empty()) {
    printf("No valid page selected from --pages.\n");
    return false;
  }

  page_indices.assign(page_set.begin(), page_set.end());
  return true;
}

WString BuildBaseName(int page_index, bool from_memory) {
  WString s = g_output_prefix;
  if (from_memory) s += L"_from_memory";
  WString page_suffix;
  page_suffix.Format((FX_LPCWSTR)L"_%d", page_index);
  s += page_suffix;
  return s;
}

void Save2Image(Bitmap& bitmap, int page_index, const char* ext, bool from_memory) {
  Image image;
  image.AddFrame(bitmap);
  WString save_path = g_output_directory + BuildBaseName(page_index, from_memory) + WString::FromLocal(ext);
  image.SaveAs(save_path);
  cout << "Save page " << page_index << " into a picture of " << ext
       << " format" << (from_memory ? " from memory" : "") << "." << endl;
}

void PDF2Image(PDFDoc doc, bool from_memory, const std::vector<int>& page_indices,
               const std::vector<std::string>& single_formats) {
  for (size_t k = 0; k < page_indices.size(); ++k) {
    int i = page_indices[k];
    PDFPage page = doc.GetPage(i);
    page.StartParse(foxit::pdf::PDFPage::e_ParsePageNormal, NULL, false);

    int width = static_cast<int>(page.GetWidth());
    int height = static_cast<int>(page.GetHeight());
    Matrix matrix = page.GetDisplayMatrix(0, 0, width, height, page.GetRotation());

    Bitmap bitmap(width, height, foxit::common::Bitmap::e_DIBArgb, NULL, 0);
    bitmap.FillRect(0xFFFFFFFF, NULL);

    Renderer render(bitmap, false);
    render.StartRender(page, matrix, NULL);
    for (size_t j = 0; j < single_formats.size(); ++j) {
      Save2Image(bitmap, i, single_formats[j].c_str(), from_memory);
    }
  }
}

void SaveTiffImage(PDFDoc doc, bool from_memory, const std::vector<int>& page_indices,
                   const std::vector<std::string>& multi_formats) {
  for (size_t i = 0; i < multi_formats.size(); ++i) {
    WString save_path = g_output_directory + g_output_prefix;
    if (from_memory) save_path += L"_from_memory";
    save_path += WString::FromLocal(multi_formats[i].c_str());

    Image image;
    for (size_t k = 0; k < page_indices.size(); ++k) {
      int page_index = page_indices[k];
      PDFPage page = doc.GetPage(page_index);
      page.StartParse(foxit::pdf::PDFPage::e_ParsePageNormal, NULL, false);

      int width = static_cast<int>(page.GetWidth());
      int height = static_cast<int>(page.GetHeight());
      Matrix matrix = page.GetDisplayMatrix(0, 0, width, height, page.GetRotation());

      Bitmap bitmap(width, height, foxit::common::Bitmap::e_DIBArgb, NULL, 0);
      bitmap.FillRect(0xFFFFFFFF, NULL);

      Renderer render(bitmap, false);
      render.StartRender(page, matrix, NULL);
      image.AddFrame(bitmap);
    }
    image.SaveAs(save_path);
    cout << "Save pdf file into a picture of " << multi_formats[i]
         << " format" << (from_memory ? " from memory" : "") << "." << endl;
  }
}

#if defined(_WIN32) || defined(_WIN64)
void SaveHDC2BMPFile(HDC hdc_display, int page_index, bool from_memory) {
  HBITMAP hBitmap = (HBITMAP)::GetCurrentObject(hdc_display, OBJ_BITMAP);
  if (!hBitmap) {
    printf("[Failed] Cannot get OBJ_BITMAP of the HDC for rendering page (page index = %d).\r\n", page_index);
    return;
  }

  BITMAP bitmap;
  ::GetObject(hBitmap, sizeof(BITMAP), &bitmap);
  unsigned int bitmap_byte_per_line = (bitmap.bmWidth * bitmap.bmBitsPixel + 7) / 8;
  bitmap_byte_per_line = (bitmap_byte_per_line + 3) / 4 * 4;
  int bitmap_byte_size = bitmap_byte_per_line * bitmap.bmHeight;

  unsigned char* hbitmap_data = new unsigned char[bitmap_byte_size];
  ::GetBitmapBits(hBitmap, bitmap_byte_size, hbitmap_data);
  if (!hbitmap_data) {
    printf("[Failed] Cannot get data of HBITMAP for page index = %d.\r\n", page_index);
    return;
  }

  String save_bmp_path;
  String prefix = String::FromUnicode(g_output_prefix);
  if (from_memory) prefix += "_from_memory";
  save_bmp_path.Format("%s%s_render_by_hdc_page%d.bmp",
    (FX_LPCSTR)String::FromUnicode(g_output_directory), (FX_LPCSTR)prefix, page_index);
  FILE* save_file = NULL;
  fopen_s(&save_file, (const char*)(FX_LPCSTR)save_bmp_path, "wb");

  if (!save_file) {
    printf("[Failed] Cannot open save file %s.\r\n", (FX_LPCSTR)save_bmp_path);
    delete[] hbitmap_data;
    return;
  }

  int file_size = 14 + 40 + bitmap_byte_size;
  unsigned char file_head_buf[14];
  memset(file_head_buf, 0, 14);
  file_head_buf[0] = 'B';
  file_head_buf[1] = 'M';
  file_head_buf[2] = (unsigned char)file_size;
  file_head_buf[3] = (unsigned char)(file_size >> 8);
  file_head_buf[4] = (unsigned char)(file_size >> 16);
  file_head_buf[5] = (unsigned char)(file_size >> 24);
  file_head_buf[10] = 54;
  fwrite(file_head_buf, 14, 1, save_file);

  unsigned char bmp_head_buf[40];
  memset(bmp_head_buf, 0, 40);
  int bitmapwidth = bitmap.bmWidth;
  int bitmapheight = bitmap.bmHeight;
  bmp_head_buf[0] = 40;
  bmp_head_buf[4] = (unsigned char)bitmapwidth;
  bmp_head_buf[5] = (unsigned char)(bitmapwidth >> 8);
  bmp_head_buf[6] = (unsigned char)(bitmapwidth >> 16);
  bmp_head_buf[7] = (unsigned char)(bitmapwidth >> 24);
  bmp_head_buf[8] = (unsigned char)(-1 * bitmapheight);
  bmp_head_buf[9] = (unsigned char)((-1 * bitmapheight) >> 8);
  bmp_head_buf[10] = (unsigned char)((-1 * bitmapheight) >> 16);
  bmp_head_buf[11] = (unsigned char)((-1 * bitmapheight) >> 24);
  bmp_head_buf[12] = 1;
  bmp_head_buf[14] = (char)bitmap.bmBitsPixel;
  fwrite(bmp_head_buf, 40, 1, save_file);

  fwrite(hbitmap_data, 1, bitmap_byte_size, save_file);
  fclose(save_file);
  delete[] hbitmap_data;
}

void PDF2ImageByDC(PDFDoc doc, const std::vector<int>& page_indices, bool from_memory) {
  for (size_t idx = 0; idx < page_indices.size(); ++idx) {
    int i = page_indices[idx];
    PDFPage page = doc.GetPage(i);
    page.StartParse(foxit::pdf::PDFPage::e_ParsePageNormal, NULL, false);

    int page_width = static_cast<int>(page.GetWidth());
    int page_height = static_cast<int>(page.GetHeight());

    HDC hdc_display = CreateDCA("DISPLAY", NULL, NULL, NULL);
    if (!hdc_display) {
      printf("[Failed] Cannot create HDC for rendering a PDF page.\r\n");
      return;
    }

    int device_width = GetDeviceCaps(hdc_display, HORZRES);
    int device_height = GetDeviceCaps(hdc_display, VERTRES);
    float x_scale = ((float)device_width) / page_width;
    float y_scale = ((float)device_height) / page_height;
    float scale = x_scale < y_scale ? x_scale : y_scale;
    int actual_render_width = 0;
    int actual_render_height = 0;
    if (x_scale < y_scale) {
      actual_render_width = device_width;
      actual_render_height = (int)(scale * page_height + 0.5);
    } else {
      actual_render_width = (int)(scale * page_width + 0.5);
      actual_render_height = device_height;
    }
    int x_offset = 0;
    int y_offset = 0;
    Matrix display_matrix = page.GetDisplayMatrix(x_offset, y_offset, actual_render_width, actual_render_height, page.GetRotation());

    HBRUSH hBrush = CreateSolidBrush(RGB(0xff, 0xff, 0xff));
    if (hBrush) {
      RECT rect = {(LONG)x_offset, (LONG)y_offset, (LONG)(actual_render_width + x_offset), (LONG)(actual_render_height + y_offset)};
      FillRect(hdc_display, &rect, hBrush);
      DeleteObject(hBrush);
    }

    Renderer render(hdc_display);
    render.StartRender(page, display_matrix, NULL);
    SaveHDC2BMPFile(hdc_display, i, from_memory);
    DeleteDC(hdc_display);
  }
}
#endif

bool RunPdf2ImageFlow(PDFDoc& doc, bool from_memory,
                      const std::vector<int>& page_indices,
                      const std::vector<std::string>& single_formats,
                      const std::vector<std::string>& multi_formats,
                      bool enable_hdc_render) {
  PDF2Image(doc, from_memory, page_indices, single_formats);
  SaveTiffImage(doc, from_memory, page_indices, multi_formats);

#if defined(_WIN32) || defined(_WIN64)
  if (enable_hdc_render) {
    DEVMODEA new_screen_settings;
    memset(&new_screen_settings, 0, sizeof(new_screen_settings));
    new_screen_settings.dmSize = sizeof(new_screen_settings);
    EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &new_screen_settings);
    if (0 != strcmp((char*)new_screen_settings.dmDeviceName, "RDPUDD")) {
      PDF2ImageByDC(doc, page_indices, from_memory);
    }
  }
#endif
  return true;
}

int main(int argc, char* argv[]) {
  Pdf2ImageCommand command;
  if (!ParseCommand(argc, argv, command)) {
    return command.show_help ? 0 : 1;
  }

  g_output_directory = command.output_directory;
  g_output_prefix = command.output_prefix;
  EnsureOutputDirectory(g_output_directory);

  int err_ret = 0;

  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    std::vector<std::string> single_formats;
    std::vector<std::string> multi_formats;
    if (!ResolveFormats(command.formats_expr, single_formats, multi_formats)) {
      return 1;
    }

    if (!command.from_memory) {
      PDFDoc doc(command.input_file);
      ErrorCode load_error = doc.Load();
      if (load_error != foxit::e_ErrSuccess) {
        printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(command.input_file), load_error);
        return 1;
      }
      std::vector<int> page_indices;
      if (!ResolvePages(command.pages_expr, doc.GetPageCount(), page_indices)) {
        return 1;
      }
      RunPdf2ImageFlow(doc, false, page_indices, single_formats, multi_formats, command.enable_hdc_render);
    } else {
      cout << "Load document from memory." << endl;
      FILE* file = NULL;
#if defined(_WIN32) || defined(_WIN64)
      fopen_s(&file, (const char*)(const char*)String::FromUnicode(command.input_file), "rb+");
#else
      file = fopen((const char*)(const char*)String::FromUnicode(command.input_file), "rb+");
#endif
      if (!file) {
        printf("Cannot open file for memory loading: %s\n", (const char*)String::FromUnicode(command.input_file));
        return 1;
      }
      fseek(file, 0, SEEK_END);
      size_t file_size = (size_t)ftell(file);
      char* buffer = (char*)malloc(file_size * sizeof(char));
      memset(buffer, 0, file_size);

      fseek(file, 0, SEEK_SET);
      fread(buffer, sizeof(char), file_size, file);
      fclose(file);

      PDFDoc doc_memory(buffer, file_size);
      ErrorCode load_error = doc_memory.Load();
      if (load_error != foxit::e_ErrSuccess) {
        printf("The Doc [%s] Error from memory: %d\n", (const char*)String::FromUnicode(command.input_file), load_error);
        free(buffer);
        return 1;
      }
      std::vector<int> page_indices;
      if (!ResolvePages(command.pages_expr, doc_memory.GetPageCount(), page_indices)) {
        free(buffer);
        return 1;
      }
      RunPdf2ImageFlow(doc_memory, true, page_indices, single_formats, multi_formats, command.enable_hdc_render);
      free(buffer);
    }
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  } catch (...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}
