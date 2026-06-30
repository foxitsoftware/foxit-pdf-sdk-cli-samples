// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to convert text to PDF document.

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
#include "../../../include/addon/conversion/fs_convert.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

struct Text2PdfCommand {
  WString input_file;
  WString output_file;
  float page_width;
  float page_height;
  WString font_name;
  WString font_file;
  int font_face_index;
  WString font_style;   // fixedpitch|serif|symbolic|script|nonsymbolic|italic|allcap|smallcap|bold (comma-separated)
  WString font_charset; // ansi|default|symbol|shiftjis|hangeul|gb2312|big5|thai|easteurope|russian|greek|turkish|hebrew|arabic|baltic
  uint32 text_color;
  float text_size;
  float line_space;
  float margin_left;
  float margin_top;
  float margin_right;
  float margin_bottom;
  bool show_help;

  Text2PdfCommand()
      : page_width(595),
        page_height(842),
        font_name(L"SimSun"),
        font_face_index(0),
        font_style(L"fixedpitch"),
        font_charset(L"gb2312"),
        text_color(0xFF0000FF),
        text_size(9),
        line_space(1),
        margin_left(0),
        margin_top(0),
        margin_right(0),
        margin_bottom(0),
        show_help(false) {}
};

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
  cout << "text2pdf --input <text_path> --output <pdf_path> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --input <path>                  Input text file path." << endl;
  cout << "  --output <path>                 Output PDF file path." << endl << endl;
  cout << "Options:" << endl;
  cout << "  --page-width <float>            Page width. Default: 595." << endl;
  cout << "  --page-height <float>           Page height. Default: 842." << endl;
  cout << "  --font-name <name>              Font name. Default: SimSun. Ignored if --font-file is set." << endl;
  cout << "  --font-file <path>              Font file path. If set, takes priority over --font-name." << endl;
  cout << "  --font-face-index <int>         Font face index (used with --font-file). Default: 0." << endl;
  cout << "  --font-style <styles>           Font style flags, comma-separated (used with --font-name):" << endl;
  cout << "                                  fixedpitch, serif, symbolic, script, nonsymbolic," << endl;
  cout << "                                  italic, allcap, smallcap, bold. Default: fixedpitch." << endl;
  cout << "  --font-charset <charset>        Font charset (used with --font-name):" << endl;
  cout << "                                  ansi, default, symbol, shiftjis, hangeul, gb2312, big5," << endl;
  cout << "                                  thai, easteurope, russian, greek, turkish, hebrew, arabic, baltic." << endl;
  cout << "                                  Default: gb2312." << endl;
  cout << "  --text-color <hex>              Text color in hex (e.g. 0xFF0000FF). Default: 0xFF0000FF." << endl;
  cout << "  --text-size <float>             Text size. Default: 9." << endl;
  cout << "  --line-space <float>            Line spacing. Default: 1." << endl;
  cout << "  --margin-left <float>           Page left margin. Default: 0." << endl;
  cout << "  --margin-top <float>            Page top margin. Default: 0." << endl;
  cout << "  --margin-right <float>          Page right margin. Default: 0." << endl;
  cout << "  --margin-bottom <float>         Page bottom margin. Default: 0." << endl;
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

bool ParseCommand(int argc, char* argv[], Text2PdfCommand& cmd) {
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
    if (key.Equal("--input")) {
      cmd.input_file = WString::FromUTF8(value);
    } else if (key.Equal("--output")) {
      cmd.output_file = WString::FromUTF8(value);
    } else if (key.Equal("--page-width")) {
      cmd.page_width = (float)atof((const char*)value);
    } else if (key.Equal("--page-height")) {
      cmd.page_height = (float)atof((const char*)value);
    } else if (key.Equal("--font-name")) {
      cmd.font_name = WString::FromUTF8(value);
    } else if (key.Equal("--font-file")) {
      cmd.font_file = WString::FromUTF8(value);
    } else if (key.Equal("--font-face-index")) {
      cmd.font_face_index = atoi((const char*)value);
    } else if (key.Equal("--font-style")) {
      cmd.font_style = WString::FromUTF8(value);
    } else if (key.Equal("--font-charset")) {
      cmd.font_charset = WString::FromUTF8(value);
    } else if (key.Equal("--text-color")) {
      cmd.text_color = (uint32)strtoul((const char*)value, NULL, 0);
    } else if (key.Equal("--text-size")) {
      cmd.text_size = (float)atof((const char*)value);
    } else if (key.Equal("--line-space")) {
      cmd.line_space = (float)atof((const char*)value);
    } else if (key.Equal("--margin-left")) {
      cmd.margin_left = (float)atof((const char*)value);
    } else if (key.Equal("--margin-top")) {
      cmd.margin_top = (float)atof((const char*)value);
    } else if (key.Equal("--margin-right")) {
      cmd.margin_right = (float)atof((const char*)value);
    } else if (key.Equal("--margin-bottom")) {
      cmd.margin_bottom = (float)atof((const char*)value);
    } else {
      cout << "Error: unknown option " << (const char*)key << endl;
      return false;
    }
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

int main(int argc, char *argv[])
{
  Text2PdfCommand cmd;
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
    foxit::addon::conversion::TXT2PDFSettingData setting_data;
    setting_data.page_width = cmd.page_width;
    setting_data.page_height = cmd.page_height;
    Font::Charset charset = Font::e_CharsetGB2312;
    String cs = String::FromUnicode(cmd.font_charset);
    if (cs.Equal("ansi"))           charset = Font::e_CharsetANSI;
    else if (cs.Equal("default"))   charset = Font::e_CharsetDefault;
    else if (cs.Equal("symbol"))    charset = Font::e_CharsetSymbol;
    else if (cs.Equal("shiftjis"))  charset = Font::e_CharsetShift_JIS;
    else if (cs.Equal("hangeul"))   charset = Font::e_CharsetHangeul;
    else if (cs.Equal("big5"))      charset = Font::e_CharsetChineseBig5;
    else if (cs.Equal("thai"))      charset = Font::e_CharsetThai;
    else if (cs.Equal("easteurope")) charset = Font::e_CharsetEastEurope;
    else if (cs.Equal("russian"))   charset = Font::e_CharsetRussian;
    else if (cs.Equal("greek"))     charset = Font::e_CharsetGreek;
    else if (cs.Equal("turkish"))   charset = Font::e_CharsetTurkish;
    else if (cs.Equal("hebrew"))    charset = Font::e_CharsetHebrew;
    else if (cs.Equal("arabic"))    charset = Font::e_CharsetArabic;
    else if (cs.Equal("baltic"))    charset = Font::e_CharsetBaltic;

    if (!cmd.font_file.IsEmpty()) {
      setting_data.font = Font((const wchar_t*)cmd.font_file, cmd.font_face_index, charset);
    } else {
      // Parse style flags (comma-separated)
      uint32 styles = 0;
      String style_str = String::FromUnicode(cmd.font_style);
      if (style_str.Find("fixedpitch") >= 0)   styles |= Font::e_StyleFixedPitch;
      if (style_str.Find("serif") >= 0)        styles |= Font::e_StyleSerif;
      if (style_str.Find("symbolic") >= 0 && style_str.Find("nonsymbolic") < 0) styles |= Font::e_StyleSymbolic;
      if (style_str.Find("nonsymbolic") >= 0)  styles |= Font::e_StyleNonSymbolic;
      if (style_str.Find("script") >= 0)       styles |= Font::e_StyleScript;
      if (style_str.Find("italic") >= 0)       styles |= Font::e_StyleItalic;
      if (style_str.Find("allcap") >= 0)       styles |= Font::e_StyleAllCap;
      if (style_str.Find("smallcap") >= 0)     styles |= Font::e_StylesSmallCap;
      if (style_str.Find("bold") >= 0)         styles |= Font::e_StylesBold;
      setting_data.font = Font((const wchar_t*)cmd.font_name, styles, charset, 0);
    }
    setting_data.text_color = cmd.text_color;
    setting_data.text_size = cmd.text_size;
    setting_data.linespace = cmd.line_space;
    setting_data.page_margin = RectF(cmd.margin_left, cmd.margin_bottom, cmd.margin_right, cmd.margin_top);
    foxit::addon::conversion::Convert::FromTXT(cmd.input_file, cmd.output_file, setting_data);
    cout << "Convert TEXT file to PDF file." << endl;
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

