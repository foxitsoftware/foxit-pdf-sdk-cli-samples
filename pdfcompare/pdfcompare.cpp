// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to compare pdf page with the other.

#include <iostream>
#include <string>

#include <time.h>
#include <map>
#include <set>
#include <cstring>

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/fs_search.h"
#include "../../../include/addon/comparison/fs_compare.h"
#include "../../../include/common/fxcrt/fx_basic.h"

using namespace std;
using namespace foxit;
using namespace common;
using namespace addon;
using foxit::common::Library;
using namespace pdf;
using namespace foxit::pdf::annots;
using namespace foxit::addon::comparison;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

struct PdfCompareCommand {
  WString base_file;
  WString compared_file;
  WString output_dir;
  WString insert_stamp_image;
  WString delete_stamp_image;
  bool show_help;

  PdfCompareCommand()
      : show_help(false) {}
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
  cout << "pdfcompare --base <base.pdf> --compared <compared.pdf> --output <output_dir>" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --base <path>                   Base PDF file path." << endl;
  cout << "  --compared <path>               Compared PDF file path." << endl;
  cout << "  --output <path>                 Output directory path." << endl << endl;
  cout << "Optional:" << endl;
  cout << "  --insert-stamp <path>           Image path for insert annotation stamp." << endl;
  cout << "  --delete-stamp <path>           Image path for delete annotation stamp." << endl;
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

bool EnsureOutputDir(const WString& dir) {
  if (dir.IsEmpty()) return false;
#if defined(_WIN32) || defined(_WIN64)
  _wmkdir((const wchar_t*)dir);
#else
  mkdir(String::FromUnicode(dir), 0777);
#endif
  return true;
}

bool ParseCommand(int argc, char* argv[], PdfCompareCommand& command) {
  if (argc <= 1) {
    PrintUsage();
    return false;
  }

  bool has_base = false;
  bool has_compared = false;
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
    if (key.Equal("--base") || key.Equal("-b")) {
      command.base_file = WString::FromUTF8(value);
      has_base = true;
    } else if (key.Equal("--compared") || key.Equal("-c")) {
      command.compared_file = WString::FromUTF8(value);
      has_compared = true;
    } else if (key.Equal("--output") || key.Equal("-o")) {
      command.output_dir = WString::FromUTF8(value);
      has_output = true;
    } else if (key.Equal("--insert-stamp")) {
      command.insert_stamp_image = WString::FromUTF8(value);
    } else if (key.Equal("--delete-stamp")) {
      command.delete_stamp_image = WString::FromUTF8(value);
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (!has_base || !has_compared || !has_output) {
    printf("--base, --compared, and --output are all required.\n");
    PrintUsage();
    return false;
  }
  if (!FileExists(command.base_file)) {
    printf("Base file does not exist: %s\n", (const char*)String::FromUnicode(command.base_file));
    return false;
  }
  if (!FileExists(command.compared_file)) {
    printf("Compared file does not exist: %s\n", (const char*)String::FromUnicode(command.compared_file));
    return false;
  }
  return true;
}

DateTime GetLocalDateTime() {
  time_t t = time(NULL);
#if (WINAPI_PARTITION_APP || WINAPI_PARTITION_PC_APP) || \
  (defined(_WIN32) || defined(_WIN64)) && _FX_COMPILER_ != _FX_VC6_
  struct tm _Tm;
  localtime_s(&_Tm, &t);
  struct tm* rime = &_Tm;
  _tzset();
  long time_zone = NULL;
  _get_timezone(&time_zone);
  int timezone_hour = time_zone / 3600 * -1;
  int timezone_minute = (abs(time_zone) % 3600) / 60;
#elif defined(__linux__)
  struct tm* rime = localtime(&t);
  tzset();
  int timezone_hour = __timezone / 3600 * -1;
  int timezone_minute = ((int)abs(__timezone) % 3600) / 60;
#elif defined(__APPLE__)
  struct tm* rime = localtime(&t);
  tzset();
  int timezone_hour = timezone / 3600 * -1;
  int timezone_minute = ((int)abs(timezone) % 3600) / 60;
#endif
  DateTime datetime;
  datetime.year = static_cast<uint16>(rime->tm_year + 1900);
  datetime.month = static_cast<uint16>(rime->tm_mon + 1);
  datetime.day = static_cast<uint16>(rime->tm_mday);
  datetime.hour = static_cast<uint16>(rime->tm_hour);
  datetime.minute = static_cast<uint16>(rime->tm_min);
  datetime.second = static_cast<uint16>(rime->tm_sec);
  datetime.utc_hour_offset = timezone_hour;
  datetime.utc_minute_offset = timezone_minute;

  return datetime;
}

String RandomUID() {
  String uuid;
  const char* c = "0123456789qwertyuiopasdfghjklzxcvbnm";

  for (int n = 0; n < 16; n++) {
    String uuid_temp;
    int b = rand() % 255;
    switch (n) {
      case 6:
        uuid_temp.Format("4%x", b % 15);
      break;
      case 8:
        uuid_temp.Format("%c%x", c[rand() % strlen(c)], b % 15);
      break;
      default:
        uuid_temp.Format("%02x", b);
      break;
    }
    uuid += uuid_temp;

    switch (n) {
      case 3:
      case 5:
      case 7:
      case 9:
        uuid += '-';
      break;
    }
  }
  return uuid;
}

void CreateInsertStamp(PDFPage page, RectFArray rects, RGB color, WString ws_contents, WString ws_type, WString ws_obj_type, const WString& image_path) {
  RectF rect_stamp;
  int rect_size = rects.GetSize();
  if (rect_size > 0) {
    RectF item = rects.GetAt(0);
    rect_stamp.left = item.left;
    rect_stamp.top = item.top-4 ;
    rect_stamp.right = rect_stamp.left + 4;
    rect_stamp.bottom = rect_stamp.top - 8;
  }

  Image image = Image(image_path);

  annots::Stamp stamp(page.AddAnnot(Annot::e_Stamp, rect_stamp));
  stamp.SetContent(ws_contents);
  stamp.SetBorderColor(color);
  stamp.SetSubject(ws_obj_type);
  stamp.SetTitle(ws_type);
  stamp.SetCreationDateTime(GetLocalDateTime());
  stamp.SetModifiedDateTime(GetLocalDateTime());
  stamp.SetUniqueID(WString::FromLocal(RandomUID()));
  stamp.SetImage(image, 0, 0);

  stamp.ResetAppearanceStream();
}

void CreateSquigglyRect(PDFPage page, RectFArray rects, RGB color, WString ws_contents, WString ws_type, WString ws_obj_type) {
  annots::Squiggly squiggly(page.AddAnnot(Annot::e_Squiggly, RectF()));
  squiggly.SetContent(ws_contents);

  annots::QuadPointsArray quad_points_array;
  int rect_size = rects.GetSize();
  for (int i=0; i<rect_size; i++) {
    CFX_FloatRect item = rects.GetAt(i);
    annots::QuadPoints quad_points;
    quad_points.first = PointF(item.left, item.top);
    quad_points.second = PointF(item.right, item.top);
    quad_points.third = PointF(item.left, item.bottom);
    quad_points.fourth = PointF(item.right, item.bottom);
    quad_points_array.Add(quad_points);
  }
  squiggly.SetQuadPoints(quad_points_array);

  squiggly.SetBorderColor(color);
  squiggly.SetSubject(ws_obj_type);
  squiggly.SetTitle(ws_type);
  squiggly.SetCreationDateTime(GetLocalDateTime());
  squiggly.SetModifiedDateTime(GetLocalDateTime());
  squiggly.SetUniqueID(WString::FromLocal(RandomUID()));

  squiggly.ResetAppearanceStream();
}

void CreateDeleteTextStamp(PDFPage page, RectFArray rects, RGB color, WString ws_contents, WString ws_type, WString ws_obj_type, const WString& image_path) {
  RectF rect_stamp;
  int rect_size = rects.GetSize();
  if (rect_size > 0) {
    RectF item = rects.GetAt(0);
    rect_stamp.left = item.left;
    rect_stamp.top = item.top + 12;
    rect_stamp.right = rect_stamp.left + 9;
    rect_stamp.bottom = rect_stamp.top - 12;
  }

  Image image = Image(image_path);

  annots::Stamp stamp(page.AddAnnot(Annot::e_Stamp, rect_stamp));
  stamp.SetContent(ws_contents);
  stamp.SetBorderColor(color);
  stamp.SetSubject(ws_obj_type);
  stamp.SetTitle(ws_type);
  stamp.SetCreationDateTime(GetLocalDateTime());
  stamp.SetModifiedDateTime(GetLocalDateTime());
  stamp.SetUniqueID(WString::FromLocal(RandomUID()));
  stamp.SetImage(image, 0, 0);

  stamp.ResetAppearanceStream();
}

void CreateDeleteText(PDFPage page, RectFArray rects, RGB color, WString ws_contents, WString ws_type, WString ws_obj_type) {
  annots::StrikeOut strikeout(page.AddAnnot(Annot::e_StrikeOut, RectF()));
  strikeout.SetContent(ws_contents);

  annots::QuadPointsArray quad_points_array;
  int rect_size = rects.GetSize();
  for (int i = 0; i<rect_size; i++) {
    CFX_FloatRect item = rects.GetAt(i);
    annots::QuadPoints quad_points;
    quad_points.first = PointF(item.left, item.top);
    quad_points.second = PointF(item.right, item.top);
    quad_points.third = PointF(item.left, item.bottom);
    quad_points.fourth = PointF(item.right, item.bottom);
    quad_points_array.Add(quad_points);
  }
  strikeout.SetQuadPoints(quad_points_array);

  strikeout.SetBorderColor(color);
  strikeout.SetSubject(ws_obj_type);
  strikeout.SetTitle(ws_type);
  strikeout.SetCreationDateTime(GetLocalDateTime());
  strikeout.SetModifiedDateTime(GetLocalDateTime());
  strikeout.SetUniqueID(WString::FromLocal(RandomUID()));

  strikeout.ResetAppearanceStream();
}

int main(int argc, char* argv[]) {
  PdfCompareCommand command;
  if (!ParseCommand(argc, argv, command)) return 1;

  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) return 1;

  // Derive resource directory from base file path as default for stamp images
  WString base_dir;
  String base_file_str = String::FromUnicode(command.base_file);
  int last_sep = base_file_str.ReverseFind('/');
#if defined(_WIN32) || defined(_WIN64)
  int last_sep_win = base_file_str.ReverseFind('\\');
  if (last_sep_win > last_sep) last_sep = last_sep_win;
#endif
  if (last_sep >= 0) {
    base_dir = WString::FromUTF8(base_file_str.Mid(0, last_sep + 1));
  }
  WString default_resource_dir = base_dir + L"pdfcompare/";
  WString insert_stamp_image = command.insert_stamp_image.IsEmpty()
      ? default_resource_dir + L"insert_stamp.png"
      : command.insert_stamp_image;
  WString delete_stamp_image = command.delete_stamp_image.IsEmpty()
      ? default_resource_dir + L"delete_stamp.png"
      : command.delete_stamp_image;

  // Ensure output directory exists
  EnsureOutputDir(command.output_dir);
  WString output_directory = command.output_dir;
  if (!output_directory.IsEmpty() && output_directory.GetAt(output_directory.GetLength() - 1) != L'/' &&
      output_directory.GetAt(output_directory.GetLength() - 1) != L'\\') {
    output_directory += L"/";
  }

  try {
    PDFDoc base_doc(command.base_file);
    error_code = base_doc.Load();
    if (error_code != foxit::e_ErrSuccess) {
      printf("Error: Load base PDF \"%s\" failed. Error code: %d\n",
             (const char*)String::FromUnicode(command.base_file), error_code);
      return 1;
    }

    PDFDoc compared_doc(command.compared_file);
    error_code = compared_doc.Load();
    if (error_code != foxit::e_ErrSuccess) {
      printf("Error: Load compared PDF \"%s\" failed. Error code: %d\n",
             (const char*)String::FromUnicode(command.compared_file), error_code);
      return 1;
    }

    Comparison comparison(base_doc, compared_doc);
    CompareResults result = comparison.DoCompare(0, 0, Comparison::e_CompareTypeText);
    CompareResultInfoArray& old_info = result.base_doc_results;
    CompareResultInfoArray& new_info = result.compared_doc_results;
    int old_info_size = old_info.GetSize();
    int new_info_size = new_info.GetSize();
    PDFPage page_base = base_doc.GetPage(0);
    PDFPage page = compared_doc.GetPage(0);
    for (int i = 0; i < old_info_size; i++) {
      const CompareResultInfo& item = old_info.GetAt(i);
      CompareResultInfo::CompareResultType type = item.type;
      if (type == CompareResultInfo::e_CompareResultTypeDeleteText) {
        String res_string;
        res_string.Format((FX_LPCSTR)"\"%s\"", (FX_LPCSTR)String::FromUnicode(item.diff_contents));
        CreateDeleteText(page_base, item.rect_array, 0xff0000, WString::FromLocal(res_string), L"Compare : Delete", L"Text");
      } else if (type == CompareResultInfo::e_CompareResultTypeInsertText) {
        String res_string;
        res_string.Format((FX_LPCSTR)"\"%s\"", (FX_LPCSTR)String::FromUnicode(item.diff_contents));
        CreateInsertStamp(page_base, item.rect_array, 0x0000ff, WString::FromLocal(res_string), L"Compare : Insert", L"Text", insert_stamp_image);
      } else if (type == CompareResultInfo::e_CompareResultTypeReplaceText) {
        String res_string;
        res_string.Format("[New]: \"%s\"\r\n[Old]: \"%s\"", (FX_LPCSTR)String::FromUnicode(new_info.GetAt(i).diff_contents), (FX_LPCSTR)String::FromUnicode(item.diff_contents));
        CreateSquigglyRect(page_base, item.rect_array, 0xe7651a, WString::FromLocal(res_string), L"Compare : Replace", L"Text");
      }
    }
    for (int i=0; i<new_info_size; i++) {
      const CompareResultInfo& item = new_info.GetAt(i);
      CompareResultInfo::CompareResultType type = item.type;
      if (type == CompareResultInfo::e_CompareResultTypeDeleteText) {
        String res_string;
        res_string.Format((FX_LPCSTR)"\"%s\"", (FX_LPCSTR)String::FromUnicode(item.diff_contents));
        CreateDeleteTextStamp(page, item.rect_array, 0xff0000, WString::FromLocal(res_string), L"Compare : Delete", L"Text", delete_stamp_image);
      } else if (type == CompareResultInfo::e_CompareResultTypeInsertText) {
        String res_string;
        res_string.Format((FX_LPCSTR)"\"%s\"", (FX_LPCSTR)String::FromUnicode(item.diff_contents));
        CreateDeleteText(page, item.rect_array, 0x0000ff, WString::FromLocal(res_string), L"Compare : Insert", L"Text");
      } else if (type == CompareResultInfo::e_CompareResultTypeReplaceText) {
        String res_string;
        res_string.Format("[Old]: \"%s\"\r\n[New]: \"%s\"", (FX_LPCSTR)String::FromUnicode(old_info.GetAt(i).diff_contents), (FX_LPCSTR)String::FromUnicode(item.diff_contents));
        CreateSquigglyRect(page, item.rect_array, 0xe7651a, WString::FromLocal(res_string), L"Compare : Replace", L"Text");
      }
    }
    base_doc.SaveAs(output_directory + L"old.pdf");
    compared_doc.SaveAs(output_directory + L"new.pdf");

    PDFDoc new_doc = comparison.GenerateComparedDoc(Comparison::e_CompareTypeAll);
    new_doc.SaveAs(output_directory + L"generate_result.pdf", PDFDoc::e_SaveFlagNormal);

    cout << "PDF compare completed. Output saved to: " << (const char*)String::FromUnicode(output_directory) << endl;
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

