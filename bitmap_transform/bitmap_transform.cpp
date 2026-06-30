// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to do bitmap transformation,
// such as flipping, stretching, and so on.

// Include Foxit SDK header files.
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cerrno>
#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/common/fs_image.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

static std::string g_operation;
static std::string g_input_file;
static std::string g_output_file;
static bool g_has_stretch_width = false;
static bool g_has_stretch_height = false;
static int g_stretch_width = 0;
static int g_stretch_height = 0;
static bool g_has_scale_x = false;
static bool g_has_scale_y = false;
static float g_scale_x = 2.0f;
static float g_scale_y = 0.5f;
static bool g_has_bbox_color = false;
static unsigned long g_bbox_color = 0xFFFDFDFD;
static bool g_has_bbox_differ_param1 = false;
static bool g_has_bbox_differ_param2 = false;
static int g_bbox_differ_param1 = 20;
static int g_bbox_differ_param2 = 64;

static void PrintUsage() {
  printf(
    "Usage: demo_bitmap_transform --op <operation> -i <input bitmap> -o <output bitmap>\n"
    "\n"
    "Required options:\n"
    "  --op, --type <name>   Operation name\n"
    "                        stretch | transform | flip | swapxy | bbox_color | bbox_differ\n"
    "  -i <file>             Input bitmap path\n"
    "  -o <file>             Output bitmap path\n"
    "\n"
    "Optional numeric options:\n"
    "  --stretch-width <n>    Output width for stretch (default: input width / 2)\n"
    "  --stretch-height <n>   Output height for stretch (default: input height * 2)\n"
    "  --scale-x <n>          X scale factor for transform (default: 2.0)\n"
    "  --scale-y <n>          Y scale factor for transform (default: 0.5)\n"
    "  --bbox-color <n>       Color value for bbox_color (default: 0xFFFDFDFD)\n"
    "  --bbox-differ-param1 <n>  First numeric parameter for bbox_differ (default: 20)\n"
    "  --bbox-differ-param2 <n>  Second numeric parameter for bbox_differ (default: 64)\n"
    "\n"
    "Examples:\n"
    "  demo_bitmap_transform --op stretch -i ./input/Foxit.bmp -o ./output/stretch.bmp\n"
    "  demo_bitmap_transform --op stretch -i ./input/Foxit.bmp -o ./output/stretch.bmp --stretch-width 300 --stretch-height 500\n"
    "  demo_bitmap_transform --op transform -i ./input/Foxit.bmp -o ./output/transform.bmp --scale-x 1.5 --scale-y 0.75\n"
    "  demo_bitmap_transform --op bbox_color -i ./input/Foxit.bmp -o ./output/bbox_color.bmp --bbox-color 0xFFFDFDFD\n"
  );
}

static bool ParseIntArg(const char* value, int* out_value) {
  char* end = NULL;
  errno = 0;
  long parsed_value = strtol(value, &end, 10);
  if (errno != 0 || end == value || *end != '\0') {
    return false;
  }
  *out_value = static_cast<int>(parsed_value);
  return true;
}

static bool ParseFloatArg(const char* value, float* out_value) {
  char* end = NULL;
  errno = 0;
  double parsed_value = strtod(value, &end);
  if (errno != 0 || end == value || *end != '\0') {
    return false;
  }
  *out_value = static_cast<float>(parsed_value);
  return true;
}

static bool ParseUnsignedLongArg(const char* value, unsigned long* out_value) {
  char* end = NULL;
  errno = 0;
  unsigned long parsed_value = strtoul(value, &end, 0);
  if (errno != 0 || end == value || *end != '\0') {
    return false;
  }
  *out_value = parsed_value;
  return true;
}

static bool ParseArgs(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    String arg(argv[i]);
    if (arg.Equal("--help")) {
      PrintUsage();
      exit(0);
    }

    if (arg.Equal("--op") || arg.Equal("--type")) {
      if (i + 1 >= argc) {
        printf("Missing value for '%s'.\n", argv[i]);
        return false;
      }
      g_operation = argv[++i];
    } else if (arg.Equal("-i")) {
      if (i + 1 >= argc) {
        printf("Missing value for '-i'.\n");
        return false;
      }
      g_input_file = argv[++i];
    } else if (arg.Equal("-o")) {
      if (i + 1 >= argc) {
        printf("Missing value for '-o'.\n");
        return false;
      }
      g_output_file = argv[++i];
    } else if (arg.Equal("--stretch-width")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--stretch-width'.\n");
        return false;
      }
      if (!ParseIntArg(argv[++i], &g_stretch_width)) {
        printf("Invalid value for '--stretch-width': %s\n", argv[i]);
        return false;
      }
      g_has_stretch_width = true;
    } else if (arg.Equal("--stretch-height")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--stretch-height'.\n");
        return false;
      }
      if (!ParseIntArg(argv[++i], &g_stretch_height)) {
        printf("Invalid value for '--stretch-height': %s\n", argv[i]);
        return false;
      }
      g_has_stretch_height = true;
    } else if (arg.Equal("--scale-x")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--scale-x'.\n");
        return false;
      }
      if (!ParseFloatArg(argv[++i], &g_scale_x)) {
        printf("Invalid value for '--scale-x': %s\n", argv[i]);
        return false;
      }
      g_has_scale_x = true;
    } else if (arg.Equal("--scale-y")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--scale-y'.\n");
        return false;
      }
      if (!ParseFloatArg(argv[++i], &g_scale_y)) {
        printf("Invalid value for '--scale-y': %s\n", argv[i]);
        return false;
      }
      g_has_scale_y = true;
    } else if (arg.Equal("--bbox-color")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--bbox-color'.\n");
        return false;
      }
      if (!ParseUnsignedLongArg(argv[++i], &g_bbox_color)) {
        printf("Invalid value for '--bbox-color': %s\n", argv[i]);
        return false;
      }
      g_has_bbox_color = true;
    } else if (arg.Equal("--bbox-differ-param1")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--bbox-differ-param1'.\n");
        return false;
      }
      if (!ParseIntArg(argv[++i], &g_bbox_differ_param1)) {
        printf("Invalid value for '--bbox-differ-param1': %s\n", argv[i]);
        return false;
      }
      g_has_bbox_differ_param1 = true;
    } else if (arg.Equal("--bbox-differ-param2")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--bbox-differ-param2'.\n");
        return false;
      }
      if (!ParseIntArg(argv[++i], &g_bbox_differ_param2)) {
        printf("Invalid value for '--bbox-differ-param2': %s\n", argv[i]);
        return false;
      }
      g_has_bbox_differ_param2 = true;
    } else {
      printf("Unknown argument: %s\n", argv[i]);
      return false;
    }
  }
  return true;
}

static bool IsValidOperation(const std::string& op) {
  return op == "stretch" || op == "transform" || op == "flip" ||
         op == "swapxy" || op == "bbox_color" || op == "bbox_differ";
}

static bool ValidateArgs() {
  if (g_operation.empty()) {
    printf("Error: --op/--type is required.\n");
    return false;
  }
  if (!IsValidOperation(g_operation)) {
    printf("Error: invalid operation '%s'.\n", g_operation.c_str());
    return false;
  }
  if (g_input_file.empty()) {
    printf("Error: -i is required.\n");
    return false;
  }
  if (g_output_file.empty()) {
    printf("Error: -o is required.\n");
    return false;
  }

  if (g_has_stretch_width && g_stretch_width <= 0) {
    printf("Error: --stretch-width must be greater than 0.\n");
    return false;
  }
  if (g_has_stretch_height && g_stretch_height <= 0) {
    printf("Error: --stretch-height must be greater than 0.\n");
    return false;
  }
  if (g_has_scale_x && g_scale_x <= 0.0f) {
    printf("Error: --scale-x must be greater than 0.\n");
    return false;
  }
  if (g_has_scale_y && g_scale_y <= 0.0f) {
    printf("Error: --scale-y must be greater than 0.\n");
    return false;
  }
  if (g_has_bbox_differ_param1 && g_bbox_differ_param1 < 0) {
    printf("Error: --bbox-differ-param1 must be greater than or equal to 0.\n");
    return false;
  }
  if (g_has_bbox_differ_param2 && g_bbox_differ_param2 < 0) {
    printf("Error: --bbox-differ-param2 must be greater than or equal to 0.\n");
    return false;
  }

  std::ifstream input_test(g_input_file.c_str(), std::ios::binary);
  if (!input_test.good()) {
    printf("Error: input file does not exist or cannot be opened: %s\n", g_input_file.c_str());
    return false;
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

static void SaveBitmap(const Bitmap& bitmap, const WString& file_path) {
  Image image;
  image.AddFrame(bitmap);
  image.SaveAs(file_path);
}

int main(int argc, char *argv[]) {
  if (!ParseArgs(argc, argv) || !ValidateArgs()) {
    PrintUsage();
    return 1;
  }

  int err_ret = 0;

  EnsureOutputDir(GetParentDir(g_output_file));

  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    WString input_file = WString::FromLocal(g_input_file.c_str());
    WString output_file = WString::FromLocal(g_output_file.c_str());

    Image image(input_file);
    Bitmap bitmap = image.GetFrameBitmap(0);

    int width = bitmap.GetWidth();
    int height = bitmap.GetHeight();

    Bitmap result;
    bool has_result = true;

    int stretch_width = g_has_stretch_width ? g_stretch_width : width / 2;
    int stretch_height = g_has_stretch_height ? g_stretch_height : height * 2;
    float scale_x = g_has_scale_x ? g_scale_x : 2.0f;
    float scale_y = g_has_scale_y ? g_scale_y : 0.5f;
    unsigned long bbox_color = g_has_bbox_color ? g_bbox_color : 0xFFFDFDFD;
    int bbox_differ_param1 = g_has_bbox_differ_param1 ? g_bbox_differ_param1 : 20;
    int bbox_differ_param2 = g_has_bbox_differ_param2 ? g_bbox_differ_param2 : 64;

    if (g_operation == "stretch") {
      result = bitmap.StretchTo(stretch_width, stretch_height, Bitmap::e_Downsample, NULL);
    } else if (g_operation == "transform") {
      int left = 0;
      int top = 0;
      result = bitmap.TransformTo(foxit::Matrix(scale_x, 0.f, 0.f, scale_y, 0.f, 0.f),
                                  Bitmap::e_Downsample, left, top, NULL);
    } else if (g_operation == "flip") {
      result = bitmap.Flip(true, true);
    } else if (g_operation == "swapxy") {
      result = bitmap.SwapXY(true, true, NULL);
    } else if (g_operation == "bbox_color") {
      RectI rect = bitmap.CalculateBBoxByColor(static_cast<foxit::uint32>(bbox_color));
      if (rect.Height() > 0 && rect.Width() > 0) {
        result = bitmap.Clone(&rect);
      } else {
        has_result = false;
        printf("Error: bbox_color produced an empty rectangle.\n");
      }
    } else if (g_operation == "bbox_differ") {
      RectI rect = bitmap.DetectBBoxByColorDiffer(bbox_differ_param1, bbox_differ_param2);
      if (rect.Height() > 0 && rect.Width() > 0) {
        result = bitmap.Clone(&rect);
      } else {
        has_result = false;
        printf("Error: bbox_differ produced an empty rectangle.\n");
      }
    }

    if (!has_result) {
      err_ret = 1;
    } else {
      SaveBitmap(result, output_file);
      cout << "Bitmap transform done: " << g_operation << " -> " << g_output_file << endl;
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