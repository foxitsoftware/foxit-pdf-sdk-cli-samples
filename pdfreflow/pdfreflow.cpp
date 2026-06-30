// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to reflow PDF pages.

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
#include "../../../include/pdf/fs_reflowpage.h"
#include "../../../include/common/fs_render.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

struct PdfReflowCommand {
  WString input_file;
  WString output_dir;
  WString mode;          // single, continuous, both (default: both)
  int zoom;              // default: 100
  float screen_width;    // default: 480
  float screen_height;   // default: 800
  float margin_left;     // default: 50
  float margin_top;      // default: 30
  float margin_right;    // default: 30
  float margin_bottom;   // default: 30
  WString parse_flags;   // normal, with-image (default: normal)
  bool show_help;
  PdfReflowCommand()
      : mode(L"both"),
        zoom(100),
        screen_width(480),
        screen_height(800),
        margin_left(50),
        margin_top(30),
        margin_right(30),
        margin_bottom(30),
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

const void SaveBitmap(Bitmap bitmap, int index, WString file_name, const WString& out_dir) {
  RectF margin = RectF(50,30,30,30);
  PointF size = PointF(480,800);
  RectI rect(0, 0, static_cast<int>(size.x), static_cast<int>(margin.top));
  bitmap.FillRect(0xFFFFFFFF, &rect);
  rect = RectI(0, static_cast<int>(size.y - margin.bottom), static_cast<int>(size.x), static_cast<int>(size.y));
  bitmap.FillRect(0xFFFFFFFF, &rect);
  Image image;
  image.AddFrame(bitmap);

  WString save_path;
  WString sIndex;
  sIndex.Format((FX_LPCWSTR)L"%d",index);
  save_path = out_dir + L"reflow" + file_name + sIndex + L".bmp";
  image.SaveAs(save_path);
}

// The change for the size of the picture depends on the size of the content of reflow page.
void ReflowSingle(PDFDoc doc, const PdfReflowCommand& cmd, const WString& out_dir) {
  RectF margin = RectF(cmd.margin_left, cmd.margin_top, cmd.margin_right, cmd.margin_bottom);
  PointF size = PointF(cmd.screen_width, cmd.screen_height);
  int page_count = doc.GetPageCount();
  for (int i=0;i<page_count;i++) {
    PDFPage page = doc.GetPage(i);
    // Parse PDF page.
    page.StartParse(foxit::pdf::PDFPage::e_ParsePageNormal, NULL, false);

    ReflowPage reflow_page(page);
    // Set some arguments used for parsing the relfow page.
    reflow_page.SetLineSpace(0);
    reflow_page.SetScreenMargin(static_cast<int>(margin.left), static_cast<int>(margin.top),
      static_cast<int>(margin.right), static_cast<int>(margin.bottom));
    reflow_page.SetScreenSize(size.x, size.y);
    reflow_page.SetZoom(cmd.zoom);
    reflow_page.SetParseFlags(ReflowPage::e_Normal);

    // Parse reflow page.
    reflow_page.StartParse(NULL);

    // Get actual size of content of reflow page. The content size does not contain the margin.
    float content_width = reflow_page.GetContentWidth();
    float content_height = reflow_page.GetContentHeight();

    // Create a bitmap for rendering the reflow page. The bitmap size contains the margin.
    Bitmap bitmap(static_cast<int>(content_width + margin.left + margin.right),
      static_cast<int>(content_height + margin.top + margin.bottom), foxit::common::Bitmap::e_DIBArgb, NULL, 0);
    bitmap.FillRect(0xFFFFFFFF, NULL);

    // Render reflow page.
    Renderer renderer(bitmap, false);
    foxit::Matrix matrix = reflow_page.GetDisplayMatrix(0, 0,content_width,content_height,Rotation::e_Rotation0);
    renderer.StartRenderReflowPage(reflow_page, matrix, NULL);
    WString file_name = L"_single_";
    SaveBitmap(bitmap, i, file_name, out_dir);
  }

}

// Fixed bitmap size, just to simulate split screen situation.
void ReflowContinuous(PDFDoc doc, const PdfReflowCommand& cmd, const WString& out_dir) {
  RectF margin = RectF(cmd.margin_left, cmd.margin_top, cmd.margin_right, cmd.margin_bottom);
  PointF size = PointF(cmd.screen_width, cmd.screen_height);
  float display_height = size.y - margin.top - margin.bottom;
  Bitmap bitmap(static_cast<int>(size.x), static_cast<int>(size.y), foxit::common::Bitmap::e_DIBArgb, NULL, 0);
  float offset_y = 0;
  int page_count = doc.GetPageCount();

  int bitmap_index = 0;

  for(int i=0;i<page_count;i++)
  {
    PDFPage page = doc.GetPage(i);

    // Parse PDF page.
    page.StartParse(foxit::pdf::PDFPage::e_ParsePageNormal, NULL, false);

    ReflowPage reflow_page(page);
    // Set some arguments used for parsing the relfow page.
    reflow_page.SetLineSpace(0);
    reflow_page.SetScreenMargin(static_cast<int>(margin.left), static_cast<int>(margin.top),
      static_cast<int>(margin.right), static_cast<int>(margin.bottom));
    reflow_page.SetScreenSize(size.x, size.y);
    reflow_page.SetZoom(cmd.zoom);
    reflow_page.SetParseFlags(ReflowPage::e_WithImage);

    reflow_page.SetTopSpace(offset_y);

    // Parse reflow page.
    reflow_page.StartParse(NULL);

    // Get actual size of content of reflow page.
    // The content size does not contain the margin but contains the top space.
    float content_height = reflow_page.GetContentHeight();
    float content_width = reflow_page.GetContentWidth();
    // Render reflow page.
    Renderer renderer(bitmap, false);
    foxit::Matrix matrix = reflow_page.GetDisplayMatrix(0, 0,(int)content_width,(int)content_height,foxit::common::Rotation::e_Rotation0);
    renderer.StartRenderReflowPage(reflow_page, matrix, NULL);

    int rate_need_screen_count = static_cast<int>(ceil(max(content_height - display_height, 0.0f) / display_height));
    if (rate_need_screen_count > 0) {
      // Before do next rendering, save current bitmap first.
      WString file_name = L"_continuous_";
      SaveBitmap(bitmap, bitmap_index++, file_name, out_dir);

      float has_display_height = display_height;
      for (int j = 0; j < rate_need_screen_count; j++) {
        // Clear the bitmap and used it to do next rendering.
        bitmap.FillRect(0xFFFFFFFF, NULL);
        // Render reflow page.
        Renderer renderer(bitmap, false);
        foxit::Matrix matrix = reflow_page.GetDisplayMatrix(0, -has_display_height,content_width,content_height,Rotation::e_Rotation0);
        renderer.StartRenderReflowPage(reflow_page, matrix, NULL);
        if (j != rate_need_screen_count - 1) {
          has_display_height += display_height;
          SaveBitmap(bitmap,  bitmap_index++, file_name, out_dir);
        } else {
          offset_y = content_height - rate_need_screen_count * display_height;
        }
      }
    } else {
      offset_y = content_height;
    }
  }
  WString file_name = L"_continuous_";
  SaveBitmap(bitmap,  bitmap_index++, file_name, out_dir);

}
void PrintUsage() {
  cout << "Usage:" << endl;
  cout << "pdfreflow --input <input.pdf> --output <output_dir> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --input <path>                  Input PDF path." << endl;
  cout << "  --output <path>                 Output directory for BMP images." << endl << endl;
  cout << "Optional:" << endl;
  cout << "  --mode <mode>                   Reflow mode: single, continuous, both (default: both)." << endl;
  cout << "  --zoom <int>                    Zoom level (default: 100)." << endl;
  cout << "  --width <float>                 Screen width (default: 480)." << endl;
  cout << "  --height <float>                Screen height (default: 800)." << endl;
  cout << "  --margin-left <float>           Left margin (default: 50)." << endl;
  cout << "  --margin-top <float>            Top margin (default: 30)." << endl;
  cout << "  --margin-right <float>          Right margin (default: 30)." << endl;
  cout << "  --margin-bottom <float>         Bottom margin (default: 30)." << endl;
  cout << "  --parse-flags <flags>           Parse flags: normal, with-image (default: normal for single, with-image for continuous)." << endl;
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

bool ParseCommand(int argc, char* argv[], PdfReflowCommand& command) {
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
      command.output_dir = WString::FromUTF8(value);
      has_output = true;
    } else if (key.Equal("--mode")) {
      command.mode = WString::FromUTF8(value);
      if (!command.mode.Equal(L"single") && !command.mode.Equal(L"continuous") && !command.mode.Equal(L"both")) {
        printf("Invalid mode: %s. Must be single, continuous, or both.\n", (const char*)value);
        return false;
      }
    } else if (key.Equal("--zoom")) {
      command.zoom = atoi(value);
    } else if (key.Equal("--width")) {
      command.screen_width = static_cast<float>(atof(value));
    } else if (key.Equal("--height")) {
      command.screen_height = static_cast<float>(atof(value));
    } else if (key.Equal("--margin-left")) {
      command.margin_left = static_cast<float>(atof(value));
    } else if (key.Equal("--margin-top")) {
      command.margin_top = static_cast<float>(atof(value));
    } else if (key.Equal("--margin-right")) {
      command.margin_right = static_cast<float>(atof(value));
    } else if (key.Equal("--margin-bottom")) {
      command.margin_bottom = static_cast<float>(atof(value));
    } else if (key.Equal("--parse-flags")) {
      command.parse_flags = WString::FromUTF8(value);
      if (!command.parse_flags.Equal(L"normal") && !command.parse_flags.Equal(L"with-image")) {
        printf("Invalid parse-flags: %s. Must be normal or with-image.\n", (const char*)value);
        return false;
      }
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
  return true;
}

int main(int argc, char *argv[])
{
  PdfReflowCommand command;
  if (!ParseCommand(argc, argv, command)) return 1;

  WString output_directory = command.output_dir;
#if defined(_WIN32) || defined(_WIN64)
  _mkdir(String::FromUnicode(output_directory));
#else
  mkdir(String::FromUnicode(output_directory), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    PDFDoc doc = PDFDoc(command.input_file);
    ErrorCode error_code = doc.Load();
    if (error_code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(command.input_file), error_code);
      return 1;
    }

    if (command.mode.Equal(L"single") || command.mode.Equal(L"both")) {
      ReflowSingle(doc, command, output_directory);
    }
    if (command.mode.Equal(L"continuous") || command.mode.Equal(L"both")) {
      ReflowContinuous(doc, command, output_directory);
    }

    cout << "Reflow test." << endl;
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

