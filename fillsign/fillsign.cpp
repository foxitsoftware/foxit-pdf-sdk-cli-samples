
// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to add  fillsign to PDF document.

// Include Foxit SDK header files.
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/fs_fillsign.h"


using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace graphics;

struct GeometryOptions {
  float x;
  float y;
  float w;
  float h;
  bool customized;

  GeometryOptions() : x(0), y(0), w(0), h(0), customized(false) {}
  GeometryOptions(float in_x, float in_y, float in_w, float in_h)
      : x(in_x), y(in_y), w(in_w), h(in_h), customized(false) {}
};

struct TextOptions {
  GeometryOptions geom;
  WString content;
  float font_size;
  float origin_x;
  float origin_y;
  float charspace;
  WString font_name;
  bool customized;

  TextOptions()
      : geom(400.0f, 100.0f, 200.0f, 200.0f),
        content(L"666777888"),
        font_size(20.0f),
        origin_x(500.0f),
        origin_y(0.0f),
        charspace(1.0f),
        font_name(L"Helvetica"),
        customized(false) {}
};

struct SignatureOptions {
  GeometryOptions geom;
  uint32 argb;
  bool customized;

  SignatureOptions() : geom(300.0f, 100.0f, 100.0f, 100.0f), argb(0xFFFF0000), customized(false) {}
};

struct CliOptions {
  WString input_file;
  WString output_file;
  String object_types_raw;
  bool show_help;

  GeometryOptions crossmark;
  GeometryOptions checkmark;
  GeometryOptions roundrect;
  GeometryOptions line;
  GeometryOptions dot;
  TextOptions text;
  SignatureOptions signature;

  CliOptions()
      : show_help(false),
        crossmark(0.0f, 0.0f, 100.0f, 100.0f),
        checkmark(100.0f, 100.0f, 50.0f, 100.0f),
        roundrect(200.0f, 200.0f, 100.0f, 50.0f),
        line(300.0f, 300.0f, 100.0f, 50.0f),
        dot(400.0f, 400.0f, 100.0f, 100.0f) {}
};

void PrintUsage() {
  cout << "Usage:" << endl
       << "demo_fillsign -i <input.pdf> -o <output.pdf> --object-types <types> [options]" << endl
       << endl
       << "Required options:" << endl
       << "  -i, --input           Input PDF path" << endl
       << "  -o, --output          Output PDF path" << endl
       << "  --object-types        Comma-separated object types: crossmark,checkmark,roundrect,line,dot,text,signature" << endl
       << endl
       << "Per-type geometry options:" << endl
       << "  --<type>-x --<type>-y --<type>-w --<type>-h" << endl
       << "  where <type> is one of crossmark/checkmark/roundrect/line/dot/text/signature" << endl
       << endl
       << "Text options:" << endl
       << "  --text-content --text-font-size --text-origin-x --text-origin-y --text-charspace" << endl       << "  --text-font          Font: standard name (Courier, CourierB, CourierBI, CourierI, Helvetica, HelveticaB," << endl
       << "                       HelveticaBI, HelveticaI, Times, TimesB, TimesBI, TimesI, Symbol, ZapfDingbats)," << endl
       << "                       PostScript name (e.g. ArialMT), or font file path (e.g. /path/to/font.ttf)" << endl       << endl
       << "Signature options:" << endl
       << "  --signature-argb      Signature bitmap color in ARGB (e.g. 0xFFFF0000)" << endl
       << endl
       << "Other:" << endl
       << "  --help                Show this help message" << endl;
}

std::string ToStdString(const String& value) {
  return std::string((const char*)value);
}

std::string ToLowerAscii(const std::string& value) {
  std::string lower = value;
  std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return lower;
}

std::string TrimAscii(const std::string& value) {
  size_t begin = 0;
  while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin]))) ++begin;
  size_t end = value.size();
  while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) --end;
  return value.substr(begin, end - begin);
}

bool ParseFloatValue(const String& value, float& result) {
  std::string text = ToStdString(value);
  char* end_ptr = NULL;
  float parsed = strtof(text.c_str(), &end_ptr);
  if (end_ptr == text.c_str() || *end_ptr != '\0') return false;
  result = parsed;
  return true;
}

bool ParseUint32Value(const String& value, uint32& result) {
  std::string text = ToStdString(value);
  char* end_ptr = NULL;
  unsigned long parsed = strtoul(text.c_str(), &end_ptr, 0);
  if (end_ptr == text.c_str() || *end_ptr != '\0') return false;
  result = static_cast<uint32>(parsed);
  return true;
}

bool SetGeometryOption(const std::string& key, const String& value, const std::string& prefix,
                       GeometryOptions& geom, std::string& error_message) {
  float parsed = 0.0f;
  if (!ParseFloatValue(value, parsed)) {
    error_message = "Invalid float value for " + key;
    return false;
  }

  if (key == "--" + prefix + "-x") geom.x = parsed;
  else if (key == "--" + prefix + "-y") geom.y = parsed;
  else if (key == "--" + prefix + "-w") geom.w = parsed;
  else if (key == "--" + prefix + "-h") geom.h = parsed;
  else return false;

  geom.customized = true;
  return true;
}

bool TryParseStandardFont(const WString& font_name, Font::StandardID& font_id) {
  std::string name = ToLowerAscii(ToStdString(String::FromUnicode(font_name)));
  if (name == "courier") font_id = Font::e_StdIDCourier;
  else if (name == "courierb") font_id = Font::e_StdIDCourierB;
  else if (name == "courierbi") font_id = Font::e_StdIDCourierBI;
  else if (name == "courieri") font_id = Font::e_StdIDCourierI;
  else if (name == "helvetica") font_id = Font::e_StdIDHelvetica;
  else if (name == "helveticab") font_id = Font::e_StdIDHelveticaB;
  else if (name == "helveticabi") font_id = Font::e_StdIDHelveticaBI;
  else if (name == "helveticai") font_id = Font::e_StdIDHelveticaI;
  else if (name == "times") font_id = Font::e_StdIDTimes;
  else if (name == "timesb") font_id = Font::e_StdIDTimesB;
  else if (name == "timesbi") font_id = Font::e_StdIDTimesBI;
  else if (name == "timesi") font_id = Font::e_StdIDTimesI;
  else if (name == "symbol") font_id = Font::e_StdIDSymbol;
  else if (name == "zapfdingbats") font_id = Font::e_StdIDZapfDingbats;
  else return false;
  return true;
}

bool FileExists(const WString& path) {
  FILE* file = NULL;
#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file, String::FromUnicode(path), "rb");
#else
  file = fopen(String::FromUnicode(path), "rb");
#endif
  if (file == NULL) return false;
  fclose(file);
  return true;
}

Font CreateFont(const WString& font_name, std::string& error_message) {
  // 1. Try standard font name
  Font::StandardID font_id;
  if (TryParseStandardFont(font_name, font_id)) {
    return Font(font_id);
  }

  // 2. Try font file path (check if file exists)
  if (FileExists(font_name)) {
    return Font((const char*)String::FromUnicode(font_name), 0, Font::e_CharsetANSI);
  }

  // 3. Use PostScript name
  return Font(font_name, Font::e_StyleNonSymbolic, Font::e_CharsetANSI, 0);
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
  if (directory.IsEmpty()) return;
#if defined(_WIN32) || defined(_WIN64)
  _mkdir(String::FromUnicode(directory));
#else
  mkdir(String::FromUnicode(directory), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

bool ParseObjectTypes(const String& raw, std::vector<std::string>& object_types, std::string& error_message) {
  std::string text = ToLowerAscii(ToStdString(raw));
  std::set<std::string> used;

  size_t start = 0;
  while (start <= text.size()) {
    size_t comma = text.find(',', start);
    std::string token = (comma == std::string::npos) ? text.substr(start) : text.substr(start, comma - start);
    token = TrimAscii(token);
    if (!token.empty()) {
      if (token != "crossmark" && token != "checkmark" && token != "roundrect" && token != "line" &&
          token != "dot" && token != "text" && token != "signature") {
        error_message = "Invalid object type: " + token;
        return false;
      }
      if (used.find(token) == used.end()) {
        used.insert(token);
        object_types.push_back(token);
      }
    }
    if (comma == std::string::npos) break;
    start = comma + 1;
  }

  if (object_types.empty()) {
    error_message = "--object-types is empty. Please provide at least one type.";
    return false;
  }
  return true;
}

bool ParseArgs(int argc, char* argv[], CliOptions& options, std::string& error_message) {
  for (int i = 1; i < argc; ++i) {
    String key_obj = String(argv[i]);
    std::string key = ToLowerAscii(ToStdString(key_obj));

    if (key == "--help") {
      options.show_help = true;
      return true;
    }

    if (i + 1 >= argc) {
      error_message = "Missing value for argument: " + key;
      return false;
    }

    String value = String(argv[++i]);
    if (key == "-i" || key == "--input") {
      options.input_file = WString::FromUTF8(value);
      continue;
    }
    if (key == "-o" || key == "--output") {
      options.output_file = WString::FromUTF8(value);
      continue;
    }
    if (key == "--object-types") {
      options.object_types_raw = value;
      continue;
    }

    if (SetGeometryOption(key, value, "crossmark", options.crossmark, error_message)) continue;
    if (SetGeometryOption(key, value, "checkmark", options.checkmark, error_message)) continue;
    if (SetGeometryOption(key, value, "roundrect", options.roundrect, error_message)) continue;
    if (SetGeometryOption(key, value, "line", options.line, error_message)) continue;
    if (SetGeometryOption(key, value, "dot", options.dot, error_message)) continue;
    if (SetGeometryOption(key, value, "text", options.text.geom, error_message)) {
      options.text.customized = true;
      continue;
    }
    if (SetGeometryOption(key, value, "signature", options.signature.geom, error_message)) {
      options.signature.customized = true;
      continue;
    }

    if (key == "--text-content") {
      options.text.content = WString::FromUTF8(value);
      options.text.customized = true;
      continue;
    }
    if (key == "--text-font-size") {
      float parsed = 0.0f;
      if (!ParseFloatValue(value, parsed)) {
        error_message = "Invalid float value for --text-font-size";
        return false;
      }
      options.text.font_size = parsed;
      options.text.customized = true;
      continue;
    }
    if (key == "--text-origin-x") {
      float parsed = 0.0f;
      if (!ParseFloatValue(value, parsed)) {
        error_message = "Invalid float value for --text-origin-x";
        return false;
      }
      options.text.origin_x = parsed;
      options.text.customized = true;
      continue;
    }
    if (key == "--text-origin-y") {
      float parsed = 0.0f;
      if (!ParseFloatValue(value, parsed)) {
        error_message = "Invalid float value for --text-origin-y";
        return false;
      }
      options.text.origin_y = parsed;
      options.text.customized = true;
      continue;
    }
    if (key == "--text-charspace") {
      float parsed = 0.0f;
      if (!ParseFloatValue(value, parsed)) {
        error_message = "Invalid float value for --text-charspace";
        return false;
      }
      options.text.charspace = parsed;
      options.text.customized = true;
      continue;
    }
    if (key == "--text-font") {
      options.text.font_name = WString::FromUTF8(value);
      options.text.customized = true;
      continue;
    }
    if (key == "--signature-argb") {
      uint32 parsed = 0;
      if (!ParseUint32Value(value, parsed)) {
        error_message = "Invalid uint32 value for --signature-argb";
        return false;
      }
      options.signature.argb = parsed;
      options.signature.customized = true;
      continue;
    }

    error_message = "Unknown argument: " + key;
    return false;
  }
  return true;
}

bool ValidateArgs(const CliOptions& options, const std::set<std::string>& selected_types, std::string& error_message) {
  if (options.input_file.IsEmpty()) {
    error_message = "Missing required argument: --input";
    return false;
  }
  if (options.output_file.IsEmpty()) {
    error_message = "Missing required argument: --output";
    return false;
  }
  if (options.object_types_raw.IsEmpty()) {
    error_message = "Missing required argument: --object-types";
    return false;
  }

  if (options.crossmark.customized && selected_types.find("crossmark") == selected_types.end()) {
    error_message = "crossmark parameters provided but crossmark is not in --object-types";
    return false;
  }
  if (options.checkmark.customized && selected_types.find("checkmark") == selected_types.end()) {
    error_message = "checkmark parameters provided but checkmark is not in --object-types";
    return false;
  }
  if (options.roundrect.customized && selected_types.find("roundrect") == selected_types.end()) {
    error_message = "roundrect parameters provided but roundrect is not in --object-types";
    return false;
  }
  if (options.line.customized && selected_types.find("line") == selected_types.end()) {
    error_message = "line parameters provided but line is not in --object-types";
    return false;
  }
  if (options.dot.customized && selected_types.find("dot") == selected_types.end()) {
    error_message = "dot parameters provided but dot is not in --object-types";
    return false;
  }
  if (options.text.customized && selected_types.find("text") == selected_types.end()) {
    error_message = "text parameters provided but text is not in --object-types";
    return false;
  }
  if (options.signature.customized && selected_types.find("signature") == selected_types.end()) {
    error_message = "signature parameters provided but signature is not in --object-types";
    return false;
  }

  return true;
}

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";
#if defined(_WIN32) || defined(_WIN64)
static WString output_path = WString::FromLocal("../output_files/");
static WString input_path = WString::FromLocal("../input_files/");
#else
static WString output_path = WString::FromLocal("./output_files/");
static WString input_path = WString::FromLocal("./input_files/");
#endif

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
  CliOptions options;
  std::string error_message;
  if (!ParseArgs(argc, argv, options, error_message)) {
    cout << error_message << endl;
    PrintUsage();
    return 1;
  }
  if (options.show_help) {
    PrintUsage();
    return 0;
  }

  std::vector<std::string> object_types;
  if (!ParseObjectTypes(options.object_types_raw, object_types, error_message)) {
    cout << error_message << endl;
    PrintUsage();
    return 1;
  }
  std::set<std::string> selected_types(object_types.begin(), object_types.end());

  if (!ValidateArgs(options, selected_types, error_message)) {
    cout << error_message << endl;
    PrintUsage();
    return 1;
  }

  if (!FileExists(options.input_file)) {
    cout << "Input file does not exist: " << String::FromUnicode(options.input_file) << endl;
    return 1;
  }
  if (FileExists(options.output_file)) {
    cout << "Output file already exists, refusing to overwrite: " << String::FromUnicode(options.output_file) << endl;
    return 1;
  }

  EnsureDirectoryExists(ParentDirectory(options.output_file));

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  cout << "fillsign Start" << endl;
  try {
    PDFDoc doc(options.input_file);

    ErrorCode load_error = doc.Load();
    if (load_error != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(options.input_file), load_error);
      return 1;
    }

    PDFPage page = doc.GetPage(0);
    page.StartParse();
    FillSign fillsign(page);

    for (size_t i = 0; i < object_types.size(); ++i) {
      const std::string& type = object_types[i];
      if (type == "crossmark") {
        FillSignObject obj = fillsign.AddObject(FillSign::e_FillSignObjectTypeCrossMark,
          PointF(options.crossmark.x, options.crossmark.y), options.crossmark.w, options.crossmark.h);
        obj.GenerateContent();
      } else if (type == "checkmark") {
        FillSignObject obj = fillsign.AddObject(FillSign::e_FillSignObjectTypeCheckMark,
          PointF(options.checkmark.x, options.checkmark.y), options.checkmark.w, options.checkmark.h);
        obj.GenerateContent();
      } else if (type == "roundrect") {
        FillSignObject obj = fillsign.AddObject(FillSign::e_FillSignObjectTypeRoundRectangle,
          PointF(options.roundrect.x, options.roundrect.y), options.roundrect.w, options.roundrect.h);
        obj.GenerateContent();
      } else if (type == "line") {
        FillSignObject obj = fillsign.AddObject(FillSign::e_FillSignObjectTypeLine,
          PointF(options.line.x, options.line.y), options.line.w, options.line.h);
        obj.GenerateContent();
      } else if (type == "dot") {
        FillSignObject obj = fillsign.AddObject(FillSign::e_FillSignObjectTypeDot,
          PointF(options.dot.x, options.dot.y), options.dot.w, options.dot.h);
        obj.GenerateContent();
      } else if (type == "text") {
        TextFillSignObjectDataArray text_array;
        TextFillSignObjectData text_data;
        TextState text_state;
        std::string font_error;
        Font font = CreateFont(options.text.font_name, font_error);

        text_state.font = font;
        text_state.font_size = options.text.font_size;
        text_state.origin_position = PointF(options.text.origin_x, options.text.origin_y);
        text_state.charspace = options.text.charspace;
        text_data.text_state = text_state;
        text_data.text = options.text.content;
        text_array.Add(text_data);

        FillSignObject obj = fillsign.AddTextObject(text_array,
          PointF(options.text.geom.x, options.text.geom.y), options.text.geom.w, options.text.geom.h);
        obj.GenerateContent();
      } else if (type == "signature") {
        FillSignObject obj = fillsign.AddObject(FillSign::e_FillSignObjectTypeSignature,
          PointF(options.signature.geom.x, options.signature.geom.y), options.signature.geom.w, options.signature.geom.h);
        Bitmap bitmap(100, 100, foxit::common::Bitmap::e_DIBArgb);
        bitmap.FillRect(options.signature.argb);
        ((SignatureFillSignObject)obj).SetBitmap(bitmap);
        obj.GenerateContent();
      }
    }

    page.GenerateContent();
    if (!doc.SaveAs(options.output_file, PDFDoc::e_SaveFlagNoOriginal)) {
      cout << "Save failed: " << String::FromUnicode(options.output_file) << endl;
      return 1;
    }

    cout << "fillsign Finish : All fillsign generated successfully" << endl;
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  }

  return err_ret;
}
