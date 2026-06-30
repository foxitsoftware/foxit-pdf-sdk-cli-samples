// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to generate barcode.

// Include Foxit SDK header files.
#include <iostream>
#include <string>
#include <cstdlib>
#include <cerrno>
#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/common/fs_barcode.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

#if defined(_WIN32) || defined(_WIN64)
static WString output_path = WString::FromLocal("../output_files/");
#else
static WString output_path = WString::FromLocal("./output_files/");
#endif
static std::string g_type;
static std::string g_output_file;
static std::string g_content;
static int g_unit_width = 2;
static int g_unit_height = 120;
static Barcode::QRErrorCorrectionLevel g_qr_level = Barcode::e_QRCorrectionLevelLow;
static bool g_has_qr_level = false;

static void PrintUsage() {
  printf(
    "Usage: demo_barcode [options]\n"
    "\n"
    "Required options:\n"
    "  --type <name>         Barcode type: code39, code128, ean8, upca, ean13, itf, pdf417, qrcode\n"
    "  -o <file>             Output image file path\n"
    "\n"
    "Optional options:\n"
    "  --content <text>      Barcode content text\n"
    "  --unit-width <n>      Unit width in pixels (preferred 1-5, default 2)\n"
    "  --unit-height <n>     Barcode height in pixels (default 120)\n"
    "  --qr-level <L|M|Q|H>  QR correction level (only valid for qrcode)\n"
    "  --help                Show this help\n"
    "\n"
    "Examples:\n"
    "  demo_barcode --type code128 -o ./output/code128.bmp --content 102030405060\n"
    "  demo_barcode --type qrcode -o ./output/qr_H.bmp --content Hello --qr-level H\n"
  );
}

static bool ParseIntArg(const char* value, int* out) {
  char* end = NULL;
  errno = 0;
  long n = strtol(value, &end, 10);
  if (errno != 0 || end == value || *end != '\0') {
    return false;
  }
  *out = static_cast<int>(n);
  return true;
}

static bool ParseQRErrorLevel(const std::string& level, Barcode::QRErrorCorrectionLevel* qr_level) {
  if (level == "L" || level == "l") {
    *qr_level = Barcode::e_QRCorrectionLevelLow;
  } else if (level == "M" || level == "m") {
    *qr_level = Barcode::e_QRCorrectionLevelMedium;
  } else if (level == "Q" || level == "q") {
    *qr_level = Barcode::e_QRCorrectionLevelQuater;
  } else if (level == "H" || level == "h") {
    *qr_level = Barcode::e_QRCorrectionLevelHigh;
  } else {
    return false;
  }
  return true;
}

static bool ParseType(const std::string& type, Barcode::Format* format) {
  if (type == "code39") {
    *format = Barcode::e_FormatCode39;
  } else if (type == "code128") {
    *format = Barcode::e_FormatCode128;
  } else if (type == "ean8") {
    *format = Barcode::e_FormatEAN8;
  } else if (type == "upca") {
    *format = Barcode::e_FormatUPCA;
  } else if (type == "ean13") {
    *format = Barcode::e_FormatEAN13;
  } else if (type == "itf") {
    *format = Barcode::e_FormatITF;
  } else if (type == "pdf417") {
    *format = Barcode::e_FormatPDF417;
  } else if (type == "qrcode") {
    *format = Barcode::e_FormatQRCode;
  } else {
    return false;
  }
  return true;
}

static WString DefaultContentForType(const std::string& type) {
  if (type == "code39") return L"TEST-SHEET";
  if (type == "code128") return L"102030405060708090";
  if (type == "ean8") return L"80674313";
  if (type == "upca") return L"890444000335";
  if (type == "ean13") return L"9780804816632";
  if (type == "itf") return L"070429";
  if (type == "pdf417") return L"Unknown - change me!";
  if (type == "qrcode") return L"TestForBarcodeQrCode";
  return L"FoxitBarcode";
}

static std::string GetParentDirectory(const std::string& path) {
  size_t sep = path.find_last_of("/\\");
  if (sep == std::string::npos) return "";
  return path.substr(0, sep);
}

static bool CreateDirectoryIfNeeded(const std::string& dir_path) {
  if (dir_path.empty()) return true;
  std::string normalized = dir_path;
  for (size_t i = 0; i < normalized.size(); ++i) {
    if (normalized[i] == '\\') normalized[i] = '/';
  }

  std::string current;
  if (!normalized.empty() && normalized[0] == '/') {
    current = "/";
  }

  size_t pos = 0;
  while (pos < normalized.size()) {
    size_t next = normalized.find('/', pos);
    std::string token = (next == std::string::npos) ? normalized.substr(pos) : normalized.substr(pos, next - pos);
    if (!token.empty()) {
      if (!current.empty() && current[current.size() - 1] != '/') current += "/";
      current += token;
#if defined(_WIN32) || defined(_WIN64)
      _mkdir(current.c_str());
#else
      mkdir(current.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    }
    if (next == std::string::npos) break;
    pos = next + 1;
  }
  return true;
}

static bool ParseArgs(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    String arg(argv[i]);
    if (arg.Equal("--help")) {
      PrintUsage();
      exit(0);
    }

    if (arg.Equal("--type")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--type'.\n");
        return false;
      }
      ++i;
      g_type = argv[i];
    } else if (arg.Equal("-o")) {
      if (i + 1 >= argc) {
        printf("Missing value for '-o'.\n");
        return false;
      }
      ++i;
      g_output_file = argv[i];
    } else if (arg.Equal("--content")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--content'.\n");
        return false;
      }
      ++i;
      g_content = argv[i];
    } else if (arg.Equal("--unit-width")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--unit-width'.\n");
        return false;
      }
      ++i;
      if (!ParseIntArg(argv[i], &g_unit_width)) {
        printf("Invalid value for '--unit-width': %s\n", argv[i]);
        return false;
      }
    } else if (arg.Equal("--unit-height")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--unit-height'.\n");
        return false;
      }
      ++i;
      if (!ParseIntArg(argv[i], &g_unit_height)) {
        printf("Invalid value for '--unit-height': %s\n", argv[i]);
        return false;
      }
    } else if (arg.Equal("--qr-level")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--qr-level'.\n");
        return false;
      }
      ++i;
      if (!ParseQRErrorLevel(argv[i], &g_qr_level)) {
        printf("Invalid value for '--qr-level': %s. Expected: L, M, Q, or H\n", argv[i]);
        return false;
      }
      g_has_qr_level = true;
    } else {
      printf("Unknown argument: %s\n", argv[i]);
      return false;
    }
  }
  return true;
}

static bool ValidateArgs() {
  if (g_type.empty()) {
    printf("Error: '--type' is required.\n");
    return false;
  }
  if (g_output_file.empty()) {
    printf("Error: '-o' output file path is required.\n");
    return false;
  }

  Barcode::Format format;
  if (!ParseType(g_type, &format)) {
    printf("Error: invalid '--type' value '%s'.\n", g_type.c_str());
    return false;
  }
  if (g_has_qr_level && format != Barcode::e_FormatQRCode) {
    printf("Error: '--qr-level' is only valid when '--type qrcode'.\n");
    return false;
  }
  if (g_unit_width <= 0) {
    printf("Error: '--unit-width' must be greater than 0.\n");
    return false;
  }
  if (g_unit_height <= 0) {
    printf("Error: '--unit-height' must be greater than 0.\n");
    return false;
  }
  return true;
}

class SdkLibMgr {
public:
  SdkLibMgr() : isInitialize(false){};
  ErrorCode Initialize() {
    ErrorCode error_code = Library::Initialize(sn, key);
    if (error_code != foxit::e_ErrSuccess) {
      printf("Library Initialize Error: %d\n", error_code);
    } else {
      isInitialize = true;
    }
    return error_code;

  }
  ~SdkLibMgr(){
    if(isInitialize)
      Library::Release();
  }
private:
  bool isInitialize;
};

static bool Save2Image(Bitmap& bitmap, const WString& output_file) {
  Image image;
  image.AddFrame(bitmap);
  return image.SaveAs(output_file);
}

static bool GenerateOne(const WString& code_string,
                        Barcode::Format code_format,
                        int unit_width,
                        int unit_height,
                        Barcode::QRErrorCorrectionLevel qr_level,
                        const WString& output_file) {
  Barcode barcode;
  Bitmap bitmap = barcode.GenerateBitmap(code_string, code_format, unit_width, unit_height, qr_level);
  return Save2Image(bitmap, output_file);
}

int main(int argc, char *argv[]) {
  if (!ParseArgs(argc, argv)) {
    PrintUsage();
    return 1;
  }
  if (!ValidateArgs()) {
    PrintUsage();
    return 1;
  }

  int err_ret = 0;

  std::string parent_dir = GetParentDirectory(g_output_file);
  if (!CreateDirectoryIfNeeded(parent_dir)) {
    printf("Error: failed to create output directory '%s'.\n", parent_dir.c_str());
    return 1;
  }

  SdkLibMgr spSdkLibMgr;
  // Initialize library.
  ErrorCode error_code = spSdkLibMgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    Barcode::Format code_format;
    if (!ParseType(g_type, &code_format)) {
      printf("Error: invalid '--type' value '%s'.\n", g_type.c_str());
      return 1;
    }

    WString code_string = g_content.empty() ? DefaultContentForType(g_type) : WString::FromUTF8(g_content.c_str());
    Barcode::QRErrorCorrectionLevel qr_level = Barcode::e_QRCorrectionLevelLow;
    if (code_format == Barcode::e_FormatQRCode) {
      qr_level = g_has_qr_level ? g_qr_level : Barcode::e_QRCorrectionLevelLow;
    }

    WString output_file = WString::FromLocal(g_output_file.c_str());
    bool ok = GenerateOne(code_string, code_format, g_unit_width, g_unit_height, qr_level, output_file);
    if (!ok) {
      printf("Error: failed to save image to '%s'.\n", g_output_file.c_str());
      return 1;
    }

    cout << "Generated barcode image: " << g_output_file << endl;

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