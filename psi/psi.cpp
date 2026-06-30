// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to creat a pressure sensitive ink
// and convert it to annotation.

#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/fs_psi.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace annots;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

struct PsiCommand {
  WString output_dir;
  WString points_file;
  int diameter;
  uint32 color;
  float opacity;
  float width;
  float height;
  bool show_help;

  PsiCommand()
      : diameter(9),
        color(0x434236),
        opacity(0.8f),
        width(480),
        height(180),
        show_help(false) {}
};

void PrintUsage() {
  cout << "Usage:" << endl;
  cout << "psi --output <directory> --points-file <path> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --output <directory>            Output directory." << endl;
  cout << "  --points-file <path>            File containing points data (groups of 4 values: type x y pressure)." << endl << endl;
  cout << "Options:" << endl;
  cout << "  --diameter <int>                Ink diameter (integer > 1). Default: 9." << endl;
  cout << "  --color <hex>                   Ink color hex. Default: 0x434236." << endl;
  cout << "  --opacity <float>               Ink opacity. Default: 0.8." << endl;
  cout << "  --width <float>                 PSI width. Default: 480." << endl;
  cout << "  --height <float>                PSI height. Default: 180." << endl;
  cout << "  --help                          Show this message." << endl;
}

uint32 ParseHexUint32(const String& value) {
  if (value.GetLength() > 2 && (value.GetAt(0) == '0' && (value.GetAt(1) == 'x' || value.GetAt(1) == 'X'))) {
    return (uint32)strtoul((const char*)value, NULL, 0);
  }
  return (uint32)atoi((const char*)value);
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

bool ParseCommand(int argc, char* argv[], PsiCommand& command) {
  if (argc <= 1) {
    PrintUsage();
    return false;
  }

  bool has_output = false;
  bool has_points_file = false;
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
    if (key.Equal("--output") || key.Equal("-o")) {
      command.output_dir = WString::FromUTF8(value);
      has_output = true;
    } else if (key.Equal("--points-file")) {
      command.points_file = WString::FromUTF8(value);
      has_points_file = true;
    } else if (key.Equal("--diameter")) {
      int d = atoi((const char*)value);
      if (d <= 1) {
        printf("--diameter must be an integer greater than 1.\n");
        return false;
      }
      command.diameter = d;
    } else if (key.Equal("--color")) {
      command.color = ParseHexUint32(value);
    } else if (key.Equal("--opacity")) {
      command.opacity = (float)atof((const char*)value);
    } else if (key.Equal("--width")) {
      command.width = (float)atof((const char*)value);
    } else if (key.Equal("--height")) {
      command.height = (float)atof((const char*)value);
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (!has_output || !has_points_file) {
    printf("--output and --points-file are required.\n");
    PrintUsage();
    return false;
  }
  if (!FileExists(command.points_file)) {
    printf("Points file does not exist: %s\n", (const char*)String::FromUnicode(command.points_file));
    return false;
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

int main(int argc, char *argv[])
{
  PsiCommand command;
  if (!ParseCommand(argc, argv, command)) {
    return 1;
  }

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
    // Load points data from file.
    std::vector<float> points_data;
    {
      FILE* pf = NULL;
#if defined(_WIN32) || defined(_WIN64)
      _wfopen_s(&pf, (const wchar_t*)command.points_file, L"r");
#else
      pf = fopen(String::FromUnicode(command.points_file), "r");
#endif
      if (!pf) {
        printf("Failed to open points file.\n");
        return 1;
      }
      float v;
      while (fscanf(pf, "%f", &v) == 1)
        points_data.push_back(v);
      fclose(pf);
    }
    if (points_data.size() % 4 != 0 || points_data.empty()) {
      printf("Points file must contain groups of 4 values: type x y pressure.\n");
      return 1;
    }

    PSI psi(command.width, command.height, true);

    // Set ink diameter.
    psi.SetDiameter(command.diameter);

    // Set ink color.
    psi.SetColor(command.color);

    // Set ink opacity.
    psi.SetOpacity(command.opacity);

    // Add points to pressure sensitive ink.
    for (size_t i = 0; i + 3 < points_data.size(); i += 4) {
      float x = points_data[i + 1] - 100.f;
      float y = points_data[i + 2] - 300.f;
      float pressure = points_data[i + 3];
      Path::PointType type = Path::PointType((int)points_data[i]);
      psi.AddPoint(PointF(x, y), type, pressure);
    }

    Image image;
    image.AddFrame(psi.GetBitmap());
    WString output_file = output_directory + L"pressure_sensitive_ink.bmp";
    image.SaveAs(output_file);

    PDFDoc doc;
    PDFPage page = doc.InsertPage(0);

    // Convert PSI to PSInk annotation.
    float width = command.width;
    float height = command.height;
    foxit::RectF rect(page.GetWidth() / 2 - width / 2, page.GetHeight() / 2 - height / 2,
      page.GetWidth() / 2 + width / 2, page.GetHeight() / 2 + height / 2);

    Annot annot = psi.ConvertToPDFAnnot(page, rect, e_Rotation0);

    output_file = output_directory + L"pressure_sensitive_ink.pdf";
    doc.SaveAs(output_file, PDFDoc::e_SaveFlagNormal);

    cout << "PSI test." << endl;

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

