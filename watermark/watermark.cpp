// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to add watermarks (in different types)
// into PDF files.

// Include Foxit SDK header files.
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include <string>
#include <iostream>

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/fs_watermark.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

struct WatermarkCommand {
  WString action;           // add or remove
  WString input_file;
  WString output_file;
  // text watermark
  WString text;
  uint32 text_color;
  float text_size;
  int text_opacity;
  float text_rotation;
  WString text_position;    // default: top-right
  float text_offset_x;
  float text_offset_y;
  float text_scale_x;
  float text_scale_y;
  WString text_flags;       // comma-separated: page-contents,annot,on-top,no-print,invisible
  WString text_alignment;   // left|center|right
  WString text_font_style;  // normal|underline
  float text_line_space;
  WString text_font_id;     // courier|courierb|courierbi|courieri|helvetica|helveticab|helveticabi|helveticai|times|timesb|timesbi|timesi|symbol|zapfdingbats
  // bitmap watermark (enabled when --bitmap-path is given)
  WString bitmap_path;
  int bitmap_opacity;
  float bitmap_rotation;
  WString bitmap_position;
  float bitmap_offset_x;
  float bitmap_offset_y;
  float bitmap_scale_x;     // -1 = auto
  float bitmap_scale_y;     // -1 = auto
  WString bitmap_flags;
  // image watermark (enabled when --image-path is given)
  WString image_path;
  int image_opacity;
  float image_rotation;
  WString image_position;
  float image_offset_x;
  float image_offset_y;
  float image_scale_x;      // -1 = auto
  float image_scale_y;      // -1 = auto
  WString image_flags;
  int image_frame_index;
  // single page watermark (enabled when --single-source-page >= 0)
  int single_source_page;   // -1 = disabled
  int single_opacity;
  float single_rotation;
  WString single_position;
  float single_offset_x;
  float single_offset_y;
  float single_scale_x;
  float single_scale_y;
  WString single_flags;
  bool show_help;

  WatermarkCommand()
      : text(L"Foxit PDF SDK\nwww.foxitsoftware.com"),
        text_color(0xF68C21), text_size(12), text_opacity(90), text_rotation(-45),
        text_position(L"top-right"), text_offset_x(0), text_offset_y(0),
        text_scale_x(1.f), text_scale_y(1.f), text_flags(L"page-contents,on-top"),
        text_alignment(L"center"), text_font_style(L"normal"), text_line_space(1),
        text_font_id(L"timesb"),
        bitmap_opacity(60), bitmap_rotation(90.f), bitmap_position(L"center-left"),
        bitmap_offset_x(0), bitmap_offset_y(0), bitmap_scale_x(-1), bitmap_scale_y(-1),
        bitmap_flags(L"page-contents,on-top"),
        image_opacity(20), image_rotation(0.f), image_position(L"center"),
        image_offset_x(0), image_offset_y(0), image_scale_x(-1), image_scale_y(-1),
        image_flags(L"page-contents,on-top"), image_frame_index(0),
        single_source_page(-1), single_opacity(90), single_rotation(0.f),
        single_position(L"bottom-right"), single_offset_x(0), single_offset_y(0),
        single_scale_x(0.1f), single_scale_y(0.1f), single_flags(L"page-contents,on-top"),
        show_help(false) {}
};

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

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

void PrintUsage() {
  cout << "Usage:" << endl;
  cout << "demo_watermark --action <add|remove> --input <pdf_path> --output <pdf_path> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --action <type>                 Action: add | remove." << endl;
  cout << "  --input <path>                  Input PDF file path." << endl;
  cout << "  --output <path>                 Output PDF file path." << endl << endl;
  cout << "Options (for add action):" << endl;
  cout << "  --text <string>                 Text watermark content. Default: \"Foxit PDF SDK\nwww.foxitsoftware.com\"." << endl;
  cout << "  --text-color <hex>              Text color in hex (e.g. 0xF68C21). Default: 0xF68C21." << endl;
  cout << "  --text-size <float>             Text font size. Default: 12." << endl;
  cout << "  --opacity <int>                 Text watermark opacity. Default: 90." << endl;
  cout << "  --rotation <float>              Text watermark rotation. Default: -45." << endl;
  cout << "  --bitmap-path <path>            Bitmap watermark file path (optional)." << endl;
  cout << "  --image-path <path>             Image watermark file path (optional)." << endl;
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

bool ParseCommand(int argc, char* argv[], WatermarkCommand& cmd) {
  for (int i = 1; i < argc; i += 2) {
    String key = String(argv[i]);
    if (key.Equal("--help")) {
      cmd.show_help = true;
      return true;
    }
    if (i + 1 >= argc) {
      cout << "Error: missing value for " << (const char*)key << endl;
      return false;
    }
    String value = String(argv[i + 1]);
    if (key.Equal("--action")) {
      cmd.action = WString::FromUTF8(value);
    } else if (key.Equal("--input")) {
      cmd.input_file = WString::FromUTF8(value);
    } else if (key.Equal("--output")) {
      cmd.output_file = WString::FromUTF8(value);
    } else if (key.Equal("--text")) {
      cmd.text = WString::FromUTF8(value);
    } else if (key.Equal("--text-color")) {
      cmd.text_color = (uint32)strtoul((const char*)value, NULL, 0);
    } else if (key.Equal("--text-size")) {
      cmd.text_size = (float)atof((const char*)value);
    } else if (key.Equal("--text-opacity")) {
      cmd.text_opacity = atoi((const char*)value);
    } else if (key.Equal("--text-rotation")) {
      cmd.text_rotation = (float)atof((const char*)value);
    } else if (key.Equal("--text-position")) {
      cmd.text_position = WString::FromUTF8(value);
    } else if (key.Equal("--text-offset-x")) {
      cmd.text_offset_x = (float)atof((const char*)value);
    } else if (key.Equal("--text-offset-y")) {
      cmd.text_offset_y = (float)atof((const char*)value);
    } else if (key.Equal("--text-scale-x")) {
      cmd.text_scale_x = (float)atof((const char*)value);
    } else if (key.Equal("--text-scale-y")) {
      cmd.text_scale_y = (float)atof((const char*)value);
    } else if (key.Equal("--text-flags")) {
      cmd.text_flags = WString::FromUTF8(value);
    } else if (key.Equal("--text-alignment")) {
      cmd.text_alignment = WString::FromUTF8(value);
    } else if (key.Equal("--text-font-style")) {
      cmd.text_font_style = WString::FromUTF8(value);
    } else if (key.Equal("--text-line-space")) {
      cmd.text_line_space = (float)atof((const char*)value);
    } else if (key.Equal("--text-font-id")) {
      cmd.text_font_id = WString::FromUTF8(value);
    } else if (key.Equal("--bitmap-path")) {
      cmd.bitmap_path = WString::FromUTF8(value);
    } else if (key.Equal("--bitmap-opacity")) {
      cmd.bitmap_opacity = atoi((const char*)value);
    } else if (key.Equal("--bitmap-rotation")) {
      cmd.bitmap_rotation = (float)atof((const char*)value);
    } else if (key.Equal("--bitmap-position")) {
      cmd.bitmap_position = WString::FromUTF8(value);
    } else if (key.Equal("--bitmap-offset-x")) {
      cmd.bitmap_offset_x = (float)atof((const char*)value);
    } else if (key.Equal("--bitmap-offset-y")) {
      cmd.bitmap_offset_y = (float)atof((const char*)value);
    } else if (key.Equal("--bitmap-scale-x")) {
      cmd.bitmap_scale_x = (float)atof((const char*)value);
    } else if (key.Equal("--bitmap-scale-y")) {
      cmd.bitmap_scale_y = (float)atof((const char*)value);
    } else if (key.Equal("--bitmap-flags")) {
      cmd.bitmap_flags = WString::FromUTF8(value);
    } else if (key.Equal("--image-path")) {
      cmd.image_path = WString::FromUTF8(value);
    } else if (key.Equal("--image-opacity")) {
      cmd.image_opacity = atoi((const char*)value);
    } else if (key.Equal("--image-rotation")) {
      cmd.image_rotation = (float)atof((const char*)value);
    } else if (key.Equal("--image-position")) {
      cmd.image_position = WString::FromUTF8(value);
    } else if (key.Equal("--image-offset-x")) {
      cmd.image_offset_x = (float)atof((const char*)value);
    } else if (key.Equal("--image-offset-y")) {
      cmd.image_offset_y = (float)atof((const char*)value);
    } else if (key.Equal("--image-scale-x")) {
      cmd.image_scale_x = (float)atof((const char*)value);
    } else if (key.Equal("--image-scale-y")) {
      cmd.image_scale_y = (float)atof((const char*)value);
    } else if (key.Equal("--image-flags")) {
      cmd.image_flags = WString::FromUTF8(value);
    } else if (key.Equal("--image-frame-index")) {
      cmd.image_frame_index = atoi((const char*)value);
    } else if (key.Equal("--single-source-page")) {
      cmd.single_source_page = atoi((const char*)value);
    } else if (key.Equal("--single-opacity")) {
      cmd.single_opacity = atoi((const char*)value);
    } else if (key.Equal("--single-rotation")) {
      cmd.single_rotation = (float)atof((const char*)value);
    } else if (key.Equal("--single-position")) {
      cmd.single_position = WString::FromUTF8(value);
    } else if (key.Equal("--single-offset-x")) {
      cmd.single_offset_x = (float)atof((const char*)value);
    } else if (key.Equal("--single-offset-y")) {
      cmd.single_offset_y = (float)atof((const char*)value);
    } else if (key.Equal("--single-scale-x")) {
      cmd.single_scale_x = (float)atof((const char*)value);
    } else if (key.Equal("--single-scale-y")) {
      cmd.single_scale_y = (float)atof((const char*)value);
    } else if (key.Equal("--single-flags")) {
      cmd.single_flags = WString::FromUTF8(value);
    } else {
      cout << "Error: unknown option " << (const char*)key << endl;
      return false;
    }
  }
  if (cmd.action.IsEmpty()) {
    cout << "Error: --action is required." << endl;
    return false;
  }
  if (!cmd.action.Equal(L"add") && !cmd.action.Equal(L"remove")) {
    cout << "Error: --action must be 'add' or 'remove'." << endl;
    return false;
  }
  if (cmd.input_file.IsEmpty()) {
    cout << "Error: --input is required." << endl;
    return false;
  }
  if (cmd.output_file.IsEmpty()) {
    cout << "Error: --output is required." << endl;
    return false;
  }
  return true;
}

CommonDefines::Position ParsePosition(const WString& s) {
  String cs = String::FromUnicode(s);
  if (cs.Equal("top-left"))      return CommonDefines::e_PosTopLeft;
  if (cs.Equal("top-center"))    return CommonDefines::e_PosTopCenter;
  if (cs.Equal("center-left"))   return CommonDefines::e_PosCenterLeft;
  if (cs.Equal("center"))        return CommonDefines::e_PosCenter;
  if (cs.Equal("center-right"))  return CommonDefines::e_PosCenterRight;
  if (cs.Equal("bottom-left"))   return CommonDefines::e_PosBottomLeft;
  if (cs.Equal("bottom-center")) return CommonDefines::e_PosBottomCenter;
  if (cs.Equal("bottom-right"))  return CommonDefines::e_PosBottomRight;
  return CommonDefines::e_PosTopRight;
}

uint32 ParseWatermarkFlags(const WString& s) {
  String cs = String::FromUnicode(s);
  uint32 flags = 0;
  if (cs.Find("page-contents") >= 0) flags |= WatermarkSettings::e_FlagASPageContents;
  if (cs.Find("annot") >= 0)        flags |= WatermarkSettings::e_FlagASAnnot;
  if (cs.Find("on-top") >= 0)       flags |= WatermarkSettings::e_FlagOnTop;
  if (cs.Find("no-print") >= 0)     flags |= WatermarkSettings::e_FlagNoPrint;
  if (cs.Find("invisible") >= 0)    flags |= WatermarkSettings::e_FlagInvisible;
  return flags;
}

Font::StandardID ParseFontStdID(const WString& s) {
  String cs = String::FromUnicode(s);
  if (cs.Equal("courier"))      return Font::e_StdIDCourier;
  if (cs.Equal("courierb"))     return Font::e_StdIDCourierB;
  if (cs.Equal("courierbi"))    return Font::e_StdIDCourierBI;
  if (cs.Equal("courieri"))     return Font::e_StdIDCourierI;
  if (cs.Equal("helvetica"))    return Font::e_StdIDHelvetica;
  if (cs.Equal("helveticab"))   return Font::e_StdIDHelveticaB;
  if (cs.Equal("helveticabi"))  return Font::e_StdIDHelveticaBI;
  if (cs.Equal("helveticai"))   return Font::e_StdIDHelveticaI;
  if (cs.Equal("times"))        return Font::e_StdIDTimes;
  if (cs.Equal("timesbi"))      return Font::e_StdIDTimesBI;
  if (cs.Equal("timesi"))       return Font::e_StdIDTimesI;
  if (cs.Equal("symbol"))       return Font::e_StdIDSymbol;
  if (cs.Equal("zapfdingbats")) return Font::e_StdIDZapfDingbats;
  return Font::e_StdIDTimesB;
}

void AddTextWatermark(PDFDoc doc, PDFPage page, const WatermarkCommand& cmd) {
  WatermarkSettings settings;
  settings.flags = ParseWatermarkFlags(cmd.text_flags);
  settings.offset_x = cmd.text_offset_x;
  settings.offset_y = cmd.text_offset_y;
  settings.opacity = cmd.text_opacity;
  settings.position = ParsePosition(cmd.text_position);
  settings.rotation = cmd.text_rotation;
  settings.scale_x = cmd.text_scale_x;
  settings.scale_y = cmd.text_scale_y;

  WatermarkTextProperties text_properties;
  String align_str = String::FromUnicode(cmd.text_alignment);
  if (align_str.Equal("left"))        text_properties.alignment = CommonDefines::e_AlignmentLeft;
  else if (align_str.Equal("right"))  text_properties.alignment = CommonDefines::e_AlignmentRight;
  else                                 text_properties.alignment = CommonDefines::e_AlignmentCenter;
  text_properties.color = cmd.text_color;
  text_properties.font_style = String::FromUnicode(cmd.text_font_style).Equal("underline")
      ? WatermarkTextProperties::e_FontStyleUnderline
      : WatermarkTextProperties::e_FontStyleNormal;
  text_properties.line_space = cmd.text_line_space;
  text_properties.font_size = cmd.text_size;
  text_properties.font = Font(ParseFontStdID(cmd.text_font_id));

  Watermark watermark(doc, cmd.text, text_properties, settings);
  watermark.InsertToPage(page);
}

void AddBitmapWatermark(PDFDoc doc, PDFPage page, const WatermarkCommand& cmd) {
  WatermarkSettings settings;
  settings.flags = ParseWatermarkFlags(cmd.bitmap_flags);
  settings.offset_x = cmd.bitmap_offset_x;
  settings.offset_y = cmd.bitmap_offset_y;
  settings.opacity = cmd.bitmap_opacity;
  settings.position = ParsePosition(cmd.bitmap_position);
  settings.rotation = cmd.bitmap_rotation;

  Image image(cmd.bitmap_path);
  Bitmap bitmap = image.GetFrameBitmap(0);
  settings.scale_x = (cmd.bitmap_scale_x > 0) ? cmd.bitmap_scale_x : page.GetHeight() * 1.0f / bitmap.GetWidth();
  settings.scale_y = (cmd.bitmap_scale_y > 0) ? cmd.bitmap_scale_y : settings.scale_x;
  Watermark watermark(doc, bitmap, settings);
  watermark.InsertToPage(page);
}

void AddImageWatermark(PDFDoc doc, PDFPage page, const WatermarkCommand& cmd) {
  WatermarkSettings settings;
  settings.flags = ParseWatermarkFlags(cmd.image_flags);
  settings.offset_x = cmd.image_offset_x;
  settings.offset_y = cmd.image_offset_y;
  settings.opacity = cmd.image_opacity;
  settings.position = ParsePosition(cmd.image_position);
  settings.rotation = cmd.image_rotation;

  Image image(cmd.image_path);
  Bitmap bitmap = image.GetFrameBitmap(cmd.image_frame_index);
  settings.scale_x = (cmd.image_scale_x > 0) ? cmd.image_scale_x : page.GetWidth() * 0.618f / bitmap.GetWidth();
  settings.scale_y = (cmd.image_scale_y > 0) ? cmd.image_scale_y : settings.scale_x;

  Watermark watermark(doc, image, cmd.image_frame_index, settings);
  watermark.InsertToPage(page);
}

void AddSingleWatermark(PDFDoc doc, PDFPage source_page, PDFPage target_page, const WatermarkCommand& cmd) {
  WatermarkSettings settings;
  settings.flags = ParseWatermarkFlags(cmd.single_flags);
  settings.offset_x = cmd.single_offset_x;
  settings.offset_y = cmd.single_offset_y;
  settings.opacity = cmd.single_opacity;
  settings.position = ParsePosition(cmd.single_position);
  settings.rotation = cmd.single_rotation;
  settings.scale_x = cmd.single_scale_x;
  settings.scale_y = cmd.single_scale_y;

  Watermark watermark(doc, source_page, settings);
  watermark.InsertToPage(target_page);
}

int main(int argc, char *argv[])
{
  WatermarkCommand cmd;
  if (!ParseCommand(argc, argv, cmd)) {
    PrintUsage();
    return 1;
  }
  if (cmd.show_help) {
    PrintUsage();
    return 0;
  }
  if (!FileExists(cmd.input_file)) {
    cout << "Error: input file not found: " << (const char*)String::FromUnicode(cmd.input_file) << endl;
    return 1;
  }

  // Derive output directory from output file path
  WString output_directory;
  for (int i = (int)cmd.output_file.GetLength() - 1; i >= 0; i--) {
    wchar_t ch = cmd.output_file.GetAt(i);
    if (ch == L'/' || ch == L'\\') {
      output_directory = cmd.output_file.Mid(0, i + 1);
      break;
    }
  }
  if (!output_directory.IsEmpty()) {
#if defined(_WIN32) || defined(_WIN64)
    _mkdir(String::FromUnicode(output_directory));
#else
    mkdir(String::FromUnicode(output_directory), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    PDFDoc doc(cmd.input_file);
    ErrorCode error_code = doc.Load();

    if (error_code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(cmd.input_file), error_code);
      return 1;
    }

    if (cmd.action.Equal(L"add")) {
      int page_count = doc.GetPageCount();
      for (int i = 0; i < page_count; i++) {
        PDFPage page = doc.GetPage(i);
        page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);

        AddTextWatermark(doc, page, cmd);
        if (!cmd.bitmap_path.IsEmpty() && FileExists(cmd.bitmap_path)) {
          AddBitmapWatermark(doc, page, cmd);
        }
        if (!cmd.image_path.IsEmpty() && FileExists(cmd.image_path)) {
          AddImageWatermark(doc, page, cmd);
        }
        if (cmd.single_source_page >= 0 && cmd.single_source_page < page_count) {
          PDFPage source_page = doc.GetPage(cmd.single_source_page);
          source_page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
          AddSingleWatermark(doc, source_page, page, cmd);
        }
      }

      doc.SaveAs(cmd.output_file, PDFDoc::e_SaveFlagNoOriginal);
      cout << "Add watermarks to PDF file." << endl;
    } else if (cmd.action.Equal(L"remove")) {
      int nCount = doc.GetPageCount();
      for (int i = 0; i < nCount; i++) {
        PDFPage page = doc.GetPage(i);
        page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
        if (page.HasWatermark()) {
          page.RemoveAllWatermarks();
        }
      }

      doc.SaveAs(cmd.output_file, PDFDoc::e_SaveFlagNoOriginal);
      cout << "Remove watermarks from PDF file." << endl;
    }

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
