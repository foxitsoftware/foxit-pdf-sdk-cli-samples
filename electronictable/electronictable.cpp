
// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to add table to PDF document.

// Include Foxit SDK header files.
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/annots/fs_annot.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/addon/tablegenerator/fs_tablegenerator.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

struct ElectronicTableCommand {
  WString output_file;
  WString data_file;
  float page_width;
  float page_height;
  bool show_help;

  ElectronicTableCommand()
      : page_width(595),
        page_height(842),
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

// Lightweight JSON parser for table data configuration
class JsonValue {
public:
  enum Type { TYPE_NULL, TYPE_BOOL, TYPE_NUMBER, TYPE_STRING, TYPE_ARRAY, TYPE_OBJECT };

  JsonValue() : type_(TYPE_NULL), bool_val_(false), number_(0.0) {}
  explicit JsonValue(bool b) : type_(TYPE_BOOL), bool_val_(b), number_(0.0) {}
  explicit JsonValue(double n) : type_(TYPE_NUMBER), bool_val_(false), number_(n) {}
  explicit JsonValue(const std::string& s) : type_(TYPE_STRING), bool_val_(false), number_(0.0), string_val_(s) {}
  explicit JsonValue(const std::vector<JsonValue>& a) : type_(TYPE_ARRAY), bool_val_(false), number_(0.0), array_val_(a) {}
  explicit JsonValue(const std::map<std::string, JsonValue>& o) : type_(TYPE_OBJECT), bool_val_(false), number_(0.0), object_val_(o) {}

  Type GetType() const { return type_; }
  bool IsNull() const { return type_ == TYPE_NULL; }
  bool IsBool() const { return type_ == TYPE_BOOL; }
  bool IsNumber() const { return type_ == TYPE_NUMBER; }
  bool IsString() const { return type_ == TYPE_STRING; }
  bool IsArray() const { return type_ == TYPE_ARRAY; }
  bool IsObject() const { return type_ == TYPE_OBJECT; }

  bool AsBool() const { return bool_val_; }
  double AsNumber() const { return number_; }
  int AsInt() const { return static_cast<int>(number_); }
  float AsFloat() const { return static_cast<float>(number_); }
  const std::string& AsString() const { return string_val_; }
  const std::vector<JsonValue>& AsArray() const { return array_val_; }
  const std::map<std::string, JsonValue>& AsObject() const { return object_val_; }

  const JsonValue& operator[](int index) const {
    if (type_ == TYPE_ARRAY && index >= 0 && index < static_cast<int>(array_val_.size()))
      return array_val_[index];
    return null_value_;
  }
  const JsonValue& operator[](const std::string& key) const {
    if (type_ == TYPE_OBJECT) {
      auto it = object_val_.find(key);
      if (it != object_val_.end()) return it->second;
    }
    return null_value_;
  }
  bool HasKey(const std::string& key) const {
    return type_ == TYPE_OBJECT && object_val_.find(key) != object_val_.end();
  }
  int ArraySize() const { return static_cast<int>(array_val_.size()); }

private:
  Type type_;
  bool bool_val_;
  double number_;
  std::string string_val_;
  std::vector<JsonValue> array_val_;
  std::map<std::string, JsonValue> object_val_;
  static JsonValue null_value_;
};

JsonValue JsonValue::null_value_;

class JsonParser {
public:
  explicit JsonParser(const std::string& json) : json_(json), pos_(0) {}

  JsonValue Parse() {
    SkipWhitespace();
    return ParseValue();
  }

private:
  void SkipWhitespace() {
    while (pos_ < json_.size() && (json_[pos_] == ' ' || json_[pos_] == '\t' ||
           json_[pos_] == '\n' || json_[pos_] == '\r'))
      pos_++;
  }

  JsonValue ParseValue() {
    SkipWhitespace();
    if (pos_ >= json_.size()) return JsonValue();
    char c = json_[pos_];
    if (c == '{') return ParseObject();
    if (c == '[') return ParseArray();
    if (c == '"') return ParseString();
    if (c == 't' || c == 'f') return ParseBool();
    if (c == 'n') return ParseNull();
    if (c == '-' || (c >= '0' && c <= '9')) return ParseNumber();
    return JsonValue();
  }

  JsonValue ParseObject() {
    pos_++;  // skip '{'
    std::map<std::string, JsonValue> obj;
    SkipWhitespace();
    if (pos_ < json_.size() && json_[pos_] == '}') { pos_++; return JsonValue(obj); }
    while (pos_ < json_.size()) {
      SkipWhitespace();
      std::string key = ParseStringContent();
      SkipWhitespace();
      if (pos_ < json_.size() && json_[pos_] == ':') pos_++;  // skip ':'
      SkipWhitespace();
      JsonValue value = ParseValue();
      obj[key] = value;
      SkipWhitespace();
      if (pos_ < json_.size() && json_[pos_] == '}') { pos_++; break; }
      if (pos_ < json_.size() && json_[pos_] == ',') pos_++;  // skip ','
    }
    return JsonValue(obj);
  }

  JsonValue ParseArray() {
    pos_++;  // skip '['
    std::vector<JsonValue> arr;
    SkipWhitespace();
    if (pos_ < json_.size() && json_[pos_] == ']') { pos_++; return JsonValue(arr); }
    while (pos_ < json_.size()) {
      arr.push_back(ParseValue());
      SkipWhitespace();
      if (pos_ < json_.size() && json_[pos_] == ']') { pos_++; break; }
      if (pos_ < json_.size() && json_[pos_] == ',') pos_++;  // skip ','
    }
    return JsonValue(arr);
  }

  std::string ParseStringContent() {
    if (pos_ < json_.size() && json_[pos_] == '"') pos_++;  // skip opening '"'
    std::string result;
    while (pos_ < json_.size() && json_[pos_] != '"') {
      if (json_[pos_] == '\\') {
        pos_++;
        if (pos_ < json_.size()) {
          switch (json_[pos_]) {
            case '"':  result += '"'; break;
            case '\\': result += '\\'; break;
            case '/':  result += '/'; break;
            case 'n':  result += '\n'; break;
            case 't':  result += '\t'; break;
            case 'r':  result += '\r'; break;
            case 'u':  pos_ += 4; result += '?'; break;  // simplified unicode
            default:   result += json_[pos_]; break;
          }
        }
      } else {
        result += json_[pos_];
      }
      pos_++;
    }
    if (pos_ < json_.size() && json_[pos_] == '"') pos_++;  // skip closing '"'
    return result;
  }

  JsonValue ParseString() {
    return JsonValue(ParseStringContent());
  }

  JsonValue ParseNumber() {
    std::string num_str;
    if (pos_ < json_.size() && json_[pos_] == '-') { num_str += '-'; pos_++; }
    while (pos_ < json_.size() && json_[pos_] >= '0' && json_[pos_] <= '9') {
      num_str += json_[pos_]; pos_++;
    }
    if (pos_ < json_.size() && json_[pos_] == '.') {
      num_str += '.'; pos_++;
      while (pos_ < json_.size() && json_[pos_] >= '0' && json_[pos_] <= '9') {
        num_str += json_[pos_]; pos_++;
      }
    }
    double val = 0;
    std::istringstream iss(num_str);
    iss >> val;
    return JsonValue(val);
  }

  JsonValue ParseBool() {
    if (pos_ + 4 <= json_.size() && json_.substr(pos_, 4) == "true") { pos_ += 4; return JsonValue(true); }
    if (pos_ + 5 <= json_.size() && json_.substr(pos_, 5) == "false") { pos_ += 5; return JsonValue(false); }
    return JsonValue();
  }

  JsonValue ParseNull() {
    if (pos_ + 4 <= json_.size() && json_.substr(pos_, 4) == "null") { pos_ += 4; return JsonValue(); }
    return JsonValue();
  }

  const std::string& json_;
  size_t pos_;
};


// Helper: parse hex color string (e.g. "FF0000" or "0xFF0000") to RGB value
RGB ParseHexColor(const std::string& hex_str) {
  std::string s = hex_str;
  if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s = s.substr(2);
  unsigned int val = 0;
  std::istringstream iss(s);
  iss >> std::hex >> val;
  return static_cast<RGB>(val);
}

// Helper: parse ARGB hex color string (e.g. "FFFFFFFF" or "0xFFFFFFFF") to ARGB value
ARGB ParseArgbColor(const std::string& hex_str) {
  std::string s = hex_str;
  if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s = s.substr(2);
  unsigned int val = 0;
  std::istringstream iss(s);
  iss >> std::hex >> val;
  return static_cast<ARGB>(val);
}

// Helper: parse alignment string to Alignment enum
common::Alignment ParseAlignment(const std::string& align_str) {
  if (align_str == "center") return common::Alignment::e_AlignmentCenter;
  if (align_str == "right") return common::Alignment::e_AlignmentRight;
  return common::Alignment::e_AlignmentLeft;
}

// Helper: parse mark_style string to CornerMarkStyle enum
foxit::pdf::RichTextStyle::CornerMarkStyle ParseMarkStyle(const std::string& mark_str) {
  if (mark_str == "superscript") return foxit::pdf::RichTextStyle::e_CornerMarkSuperscript;
  if (mark_str == "subscript") return foxit::pdf::RichTextStyle::e_CornerMarkSubscript;
  return foxit::pdf::RichTextStyle::e_CornerMarkNone;
}

// Helper: parse font name to Font object
common::Font ParseFont(const std::string& font_name) {
  if (font_name.empty() || font_name == "Helvetica") return common::Font(common::Font::e_StdIDHelvetica);
#if defined(__linux__)
  if (font_name == "Times New Roman") return Font(L"FreeSerif", 0, Font::e_CharsetANSI, 0);
#else
  if (font_name == "Times New Roman") return Font(L"Times New Roman", 0, Font::e_CharsetANSI, 0);
#endif
  return Font(WString::FromUTF8(String(font_name.c_str())), 0, Font::e_CharsetANSI, 0);
}

// Helper: build RichTextStyle from JSON cell object
foxit::pdf::RichTextStyle BuildTextStyle(const JsonValue& cell_json) {
  foxit::pdf::RichTextStyle style;
  style.font = cell_json.HasKey("font") ? ParseFont(cell_json["font"].AsString()) : common::Font(common::Font::e_StdIDHelvetica);
  style.text_size = cell_json.HasKey("font_size") ? cell_json["font_size"].AsFloat() : 10.0f;
  style.text_alignment = cell_json.HasKey("alignment") ? ParseAlignment(cell_json["alignment"].AsString()) : common::Alignment::e_AlignmentLeft;
  style.text_color = cell_json.HasKey("text_color") ? ParseHexColor(cell_json["text_color"].AsString()) : 0x000000;
  style.is_bold = cell_json.HasKey("bold") ? cell_json["bold"].AsBool() : false;
  style.is_italic = cell_json.HasKey("italic") ? cell_json["italic"].AsBool() : false;
  style.is_underline = cell_json.HasKey("underline") ? cell_json["underline"].AsBool() : false;
  style.is_strikethrough = cell_json.HasKey("strikethrough") ? cell_json["strikethrough"].AsBool() : false;
  style.mark_style = cell_json.HasKey("mark_style") ? ParseMarkStyle(cell_json["mark_style"].AsString()) : foxit::pdf::RichTextStyle::e_CornerMarkNone;
  return style;
}

void PrintUsage() {
  cout << "Usage:" << endl;
  cout << "demo_electronictable --output <pdf_path> --data <json_path> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --output <path>                 Output PDF file path." << endl;
  cout << "  --data <path>                   JSON data file path defining table content." << endl << endl;
  cout << "Options:" << endl;
  cout << "  --page-width <float>            Page width in points. Default: 595." << endl;
  cout << "  --page-height <float>           Page height in points. Default: 842." << endl;
  cout << "  --help                          Show this message." << endl << endl;
  cout << "JSON format example:" << endl;
  cout << "  { \"tables\": [ { \"rect\": {\"left\":100,\"bottom\":550,\"right\":495,\"top\":742}," << endl;
  cout << "    \"rows\": 4, \"cols\": 3, \"border_width\": 1," << endl;
  cout << "    \"cells\": [ [ {\"text\":\"...\", \"alignment\":\"left\", \"font_size\":10," << endl;
  cout << "    \"text_color\":\"000000\", \"bold\":false, \"italic\":false," << endl;
  cout << "    \"underline\":false, \"strikethrough\":false, \"mark_style\":\"none\"," << endl;
  cout << "    \"fill_color\":\"FFFFFFFF\", \"font\":\"Helvetica\" } ] ] } ] }" << endl;
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

bool ParseCommand(int argc, char* argv[], ElectronicTableCommand& cmd) {
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
    if (key.Equal("--output")) {
      cmd.output_file = WString::FromUTF8(value);
    } else if (key.Equal("--data")) {
      cmd.data_file = WString::FromUTF8(value);
    } else if (key.Equal("--page-width")) {
      cmd.page_width = static_cast<float>(atof((const char*)value));
    } else if (key.Equal("--page-height")) {
      cmd.page_height = static_cast<float>(atof((const char*)value));
    } else {
      cout << "Error: unknown option " << (const char*)key << endl;
      return false;
    }
  }
  return true;
}

// Build a single table from JSON and add it to the page
bool AddTableFromJSON(PDFPage& page, const JsonValue& table_json) {
  using namespace foxit::addon::tablegenerator;

  // Parse rect
  RectF rect;
  if (table_json.HasKey("rect")) {
    const JsonValue& r = table_json["rect"];
    rect.left = r.HasKey("left") ? r["left"].AsFloat() : 100.0f;
    rect.bottom = r.HasKey("bottom") ? r["bottom"].AsFloat() : 550.0f;
    rect.right = r.HasKey("right") ? r["right"].AsFloat() : page.GetWidth() - 100.0f;
    rect.top = r.HasKey("top") ? r["top"].AsFloat() : page.GetHeight() - 100.0f;
  } else {
    rect = RectF(100, 550, page.GetWidth() - 100, page.GetHeight() - 100);
  }

  int rows = table_json.HasKey("rows") ? table_json["rows"].AsInt() : 1;
  int cols = table_json.HasKey("cols") ? table_json["cols"].AsInt() : 1;

  // Parse border info
  float border_width = table_json.HasKey("border_width") ? table_json["border_width"].AsFloat() : 1.0f;
  ARGB border_color = table_json.HasKey("border_color") ? ParseArgbColor(table_json["border_color"].AsString()) : 0xFF000000;

  TableBorderInfo outside_border_left, outside_border_right, outside_border_top, outside_border_bottom;
  TableBorderInfo inside_border_row, inside_border_col;
  outside_border_left.line_width = border_width;  outside_border_left.color = border_color;
  outside_border_right.line_width = border_width; outside_border_right.color = border_color;
  outside_border_top.line_width = border_width;   outside_border_top.color = border_color;
  outside_border_bottom.line_width = border_width; outside_border_bottom.color = border_color;
  inside_border_row.line_width = border_width;    inside_border_row.color = border_color;
  inside_border_col.line_width = border_width;    inside_border_col.color = border_color;

  // Parse row_heights and col_widths
  FloatArray row_heights, col_widths;
  if (table_json.HasKey("row_heights")) {
    const JsonValue& rh = table_json["row_heights"];
    for (int i = 0; i < rh.ArraySize(); i++) row_heights.Add(rh[i].AsFloat());
  }
  if (table_json.HasKey("col_widths")) {
    const JsonValue& cw = table_json["col_widths"];
    for (int i = 0; i < cw.ArraySize(); i++) col_widths.Add(cw[i].AsFloat());
  }

  // Parse cells
  TableCellDataArray cell_array;
  if (table_json.HasKey("cells")) {
    const JsonValue& cells = table_json["cells"];
    for (int r = 0; r < cells.ArraySize() && r < rows; r++) {
      const JsonValue& row = cells[r];
      TableCellDataColArray col_array;
      for (int c = 0; c < row.ArraySize() && c < cols; c++) {
        const JsonValue& cell = row[c];
        foxit::pdf::RichTextStyle style = BuildTextStyle(cell);
        WString cell_text = cell.HasKey("text") ? WString::FromUTF8(String(cell["text"].AsString().c_str())) : L" ";
        ARGB fill_color = cell.HasKey("fill_color") ? ParseArgbColor(cell["fill_color"].AsString()) : 0xFFFFFFFF;
        TableCellData cell_data(style, fill_color, cell_text, foxit::common::Image((FS_HANDLE)NULL), RectF());
        col_array.Add(cell_data);
      }
      // Fill remaining columns with empty cells if JSON has fewer columns
      for (int c = row.ArraySize(); c < cols; c++) {
        TableCellData cell_data(foxit::pdf::RichTextStyle(), 0xFFFFFFFF, L" ", foxit::common::Image((FS_HANDLE)NULL), RectF());
        col_array.Add(cell_data);
      }
      cell_array.Add(col_array);
    }
    // Fill remaining rows with empty cells if JSON has fewer rows
    for (int r = cells.ArraySize(); r < rows; r++) {
      TableCellDataColArray col_array;
      for (int c = 0; c < cols; c++) {
        TableCellData cell_data(foxit::pdf::RichTextStyle(), 0xFFFFFFFF, L" ", foxit::common::Image((FS_HANDLE)NULL), RectF());
        col_array.Add(cell_data);
      }
      cell_array.Add(col_array);
    }
  } else {
    // No cells data, fill with empty cells
    for (int r = 0; r < rows; r++) {
      TableCellDataColArray col_array;
      for (int c = 0; c < cols; c++) {
        TableCellData cell_data(foxit::pdf::RichTextStyle(), 0xFFFFFFFF, L" ", foxit::common::Image((FS_HANDLE)NULL), RectF());
        col_array.Add(cell_data);
      }
      cell_array.Add(col_array);
    }
  }

  TableData data(rect, rows, cols, outside_border_left, outside_border_right,
    outside_border_top, outside_border_bottom, inside_border_row, inside_border_col,
    TableCellIndexArray(), row_heights, col_widths);
  TableGenerator::AddTableToPage(page, data, cell_array);
  return true;
}


int main(int argc, char *argv[]) {
  int err_ret = 0;
  ElectronicTableCommand cmd;

  if (!ParseCommand(argc, argv, cmd)) {
    PrintUsage();
    return 1;
  }
  if (cmd.show_help) {
    PrintUsage();
    return 0;
  }
  if (cmd.output_file.IsEmpty() || cmd.data_file.IsEmpty()) {
    cout << "Error: --output and --data are required." << endl;
    PrintUsage();
    return 1;
  }
  if (!FileExists(cmd.data_file)) {
    cout << "Error: data file not found: " << (const char*)String::FromUnicode(cmd.data_file) << endl;
    return 1;
  }

  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    // Read JSON data file
    std::ifstream ifs(String::FromUnicode(cmd.data_file));
    if (!ifs.is_open()) {
      cout << "Error: cannot open data file." << endl;
      return 1;
    }
    std::string json_content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    JsonParser parser(json_content);
    JsonValue root = parser.Parse();

    // Create output directory
    WString output_directory;
    int last_sep = -1;
    WString path = cmd.output_file;
    for (int i = path.GetLength() - 1; i >= 0; i--) {
      wchar_t ch = path.GetAt(i);
      if (ch == L'/' || ch == L'\\') { last_sep = i; break; }
    }
    if (last_sep >= 0) output_directory = path.Mid(0, last_sep + 1);

#if defined(_WIN32) || defined(_WIN64)
    if (!output_directory.IsEmpty()) _mkdir(String::FromUnicode(output_directory));
#else
    if (!output_directory.IsEmpty()) mkdir(String::FromUnicode(output_directory), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

    // Create PDF document with specified page size
    PDFDoc doc = PDFDoc();
    PDFPage page = doc.InsertPage(0, cmd.page_width, cmd.page_height);

    // Add tables from JSON
    if (root.HasKey("tables")) {
      const JsonValue& tables = root["tables"];
      for (int i = 0; i < tables.ArraySize(); i++) {
        AddTableFromJSON(page, tables[i]);
      }
    } else {
      // Single table mode: the root object itself is a table definition
      AddTableFromJSON(page, root);
    }

    // Save PDF file
    doc.SaveAs(cmd.output_file, PDFDoc::e_SaveFlagNoOriginal);
    std::cout << "Electronic table demo: output saved to " << (const char*)String::FromUnicode(cmd.output_file) << std::endl;
  }
  catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  }

  return err_ret;
}
