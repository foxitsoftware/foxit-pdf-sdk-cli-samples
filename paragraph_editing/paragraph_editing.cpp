// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to edit paragraph in a PDF document.

#include <time.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
#include <cstdlib>
#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/addon/pageeditor/fs_paragraphediting.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace foxit::addon::pageeditor;

struct DemoPoint {
  float x;
  float y;
  DemoPoint() : x(0), y(0) {}
  DemoPoint(float point_x, float point_y) : x(point_x), y(point_y) {}
};

struct ParagraphEditingCommand {
  WString input_file;
  WString output_dir;
  vector<string> ops;
  int page_index;
  WString save_prefix;

  DemoPoint insert_point;
  DemoPoint select_start;
  DemoPoint select_end;
  WString insert_text;
  float font_size;
  bool underline;

  DemoPoint split_point;
  vector<DemoPoint> join_points;

  bool show_help;

  ParagraphEditingCommand()
    : page_index(0)
    , save_prefix(L"ParagraphEditing")
    , insert_point(95.0f, 728.0f)
    , select_start(95.0f, 728.0f)
    , select_end(99.0f, 728.0f)
    , insert_text(L"InsertText_Paragraph_editing")
    , font_size(24.0f)
    , underline(true)
    , split_point(289.0f, 659.0f)
    , show_help(false) {
    join_points.push_back(DemoPoint(307.0f, 637.0f));
    join_points.push_back(DemoPoint(307.0f, 453.0f));
  }
};

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

#if defined(_WIN32) || defined(_WIN64)
static WString output_path = WString::FromLocal("../output_files/");
static WString input_path = WString::FromLocal("../input_files/");
#else
static WString output_path = WString::FromLocal("./output_files/");
static WString input_path = WString::FromLocal("./input_files/");
#endif

void PrintUsage() {
  cout << "Usage:" << endl;
  cout << "paragraph_editing --input <input.pdf> --output-dir <directory> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --input <path>                 Input PDF path." << endl;
  cout << "  --output-dir <path>            Output directory path." << endl << endl;
  cout << "Optional:" << endl;
  cout << "  --ops <list>                   Comma-separated operations: paragraph,split,join." << endl;
  cout << "  --page-index <int>             Page index (default: 0)." << endl;
  cout << "  --save-prefix <text>           Output file prefix (default: ParagraphEditing)." << endl;
  cout << "  --insert-point <x,y>           Insert cursor point (top-origin coordinates)." << endl;
  cout << "  --select-start <x,y>           Selection start point (top-origin coordinates)." << endl;
  cout << "  --select-end <x,y>             Selection end point (top-origin coordinates)." << endl;
  cout << "  --insert-text <text>           Text inserted by paragraph operation." << endl;
  cout << "  --font-size <float>            Inserted text font size." << endl;
  cout << "  --underline <bool>             true/false." << endl;
  cout << "  --split-point <x,y>            Split operation click point (top-origin coordinates)." << endl;
  cout << "  --join-points <x1,y1;x2,y2;..> Join click points, semicolon-separated (top-origin coordinates)." << endl;
  cout << "                                 At least 2 points required. Merges multiple adjacent text blocks." << endl;
  cout << "  --help                         Show this message." << endl;
}

bool ParseBoolValue(const String& value, bool& out_value) {
  string text = (const char*)value;
  for (size_t i = 0; i < text.size(); ++i) {
    text[i] = (char)tolower((unsigned char)text[i]);
  }
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

bool ParsePointValue(const String& value, DemoPoint& point) {
  string text = (const char*)value;
  size_t comma = text.find(',');
  if (comma == string::npos) return false;
  string left = text.substr(0, comma);
  string right = text.substr(comma + 1);
  if (left.empty() || right.empty()) return false;
  point.x = (float)atof(left.c_str());
  point.y = (float)atof(right.c_str());
  return true;
}

bool ParseOps(const String& value, vector<string>& ops) {
  string text = (const char*)value;
  stringstream stream(text);
  string token;
  while (getline(stream, token, ',')) {
    for (size_t i = 0; i < token.size(); ++i) {
      token[i] = (char)tolower((unsigned char)token[i]);
    }
    if (token.empty()) continue;
    if (token != "paragraph" && token != "split" && token != "join") {
      return false;
    }
    ops.push_back(token);
  }
  return !ops.empty();
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

WString NormalizeOutputDir(const WString& dir_path) {
  if (dir_path.IsEmpty()) return dir_path;
  string raw = (const char*)String::FromUnicode(dir_path);
  if (!raw.empty() && raw[raw.size() - 1] != '/' && raw[raw.size() - 1] != '\\') {
#if defined(_WIN32) || defined(_WIN64)
    raw += "\\";
#else
    raw += "/";
#endif
  }
  return WString::FromUTF8(raw.c_str());
}

void EnsureOutputDir(const WString& output_dir) {
#if defined(_WIN32) || defined(_WIN64)
  _mkdir(String::FromUnicode(output_dir));
#else
  mkdir(String::FromUnicode(output_dir), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

bool ParseCommand(int argc, char* argv[], ParagraphEditingCommand& command) {
  if (argc <= 1) {
    PrintUsage();
    return false;
  }

  bool has_input = false;
  bool has_output_dir = false;
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
      command.output_dir = WString::FromUTF8(value);
      has_output_dir = true;
    } else if (key.Equal("--ops")) {
      command.ops.clear();
      if (!ParseOps(value, command.ops)) {
        printf("Invalid --ops value: %s\n", (const char*)value);
        return false;
      }
    } else if (key.Equal("--page-index")) {
      command.page_index = atoi((const char*)value);
    } else if (key.Equal("--save-prefix")) {
      command.save_prefix = WString::FromUTF8(value);
    } else if (key.Equal("--insert-point")) {
      if (!ParsePointValue(value, command.insert_point)) {
        printf("Invalid point format for --insert-point, expected x,y\n");
        return false;
      }
    } else if (key.Equal("--select-start")) {
      if (!ParsePointValue(value, command.select_start)) {
        printf("Invalid point format for --select-start, expected x,y\n");
        return false;
      }
    } else if (key.Equal("--select-end")) {
      if (!ParsePointValue(value, command.select_end)) {
        printf("Invalid point format for --select-end, expected x,y\n");
        return false;
      }
    } else if (key.Equal("--insert-text")) {
      command.insert_text = WString::FromUTF8(value);
    } else if (key.Equal("--font-size")) {
      command.font_size = (float)atof((const char*)value);
    } else if (key.Equal("--underline")) {
      if (!ParseBoolValue(value, command.underline)) {
        printf("Invalid bool value for --underline: %s\n", (const char*)value);
        return false;
      }
    } else if (key.Equal("--split-point")) {
      if (!ParsePointValue(value, command.split_point)) {
        printf("Invalid point format for --split-point, expected x,y\n");
        return false;
      }
    } else if (key.Equal("--join-points")) {
      command.join_points.clear();
      string text = (const char*)value;
      stringstream ss(text);
      string token;
      while (getline(ss, token, ';')) {
        DemoPoint pt;
        if (!ParsePointValue(String(token.c_str()), pt)) {
          printf("Invalid point format in --join-points, expected x,y separated by semicolons\n");
          return false;
        }
        command.join_points.push_back(pt);
      }
      if (command.join_points.size() < 2) {
        printf("--join-points requires at least 2 points\n");
        return false;
      }
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (!has_input || !has_output_dir) {
    printf("Both --input and --output-dir are required.\n");
    PrintUsage();
    return false;
  }
  if (!FileExists(command.input_file)) {
    printf("Input file does not exist: %s\n", (const char*)String::FromUnicode(command.input_file));
    return false;
  }
  if (command.page_index < 0) {
    printf("--page-index must be >= 0.\n");
    return false;
  }
  if (command.font_size <= 0.0f) {
    printf("--font-size must be > 0.\n");
    return false;
  }
  if (command.ops.empty()) {
    command.ops.push_back("paragraph");
    command.ops.push_back("split");
    command.ops.push_back("join");
  }

  command.output_dir = NormalizeOutputDir(command.output_dir);
  return true;
}

PointF ToPagePoint(float page_height, const DemoPoint& top_origin_point) {
  return PointF(top_origin_point.x, page_height - top_origin_point.y);
}

bool SaveStepOutput(PDFDoc& doc, const WString& output_dir, const WString& prefix, const WString& suffix) {
  WString save_pdf_path = output_dir + prefix + suffix + L".pdf";
  bool result = doc.SaveAs(save_pdf_path, PDFDoc::e_SaveFlagNoOriginal);
  if (!result) {
    printf("Failed to save file: %s\n", (const char*)String::FromUnicode(save_pdf_path));
    return false;
  }
  return true;
}

bool ExecuteParagraphOperation(ParagraphEditing& paragraph_editing, float page_height, const ParagraphEditingCommand& command) {
  paragraph_editing.Activate();
  paragraph_editing.StartEditing(0, ToPagePoint(page_height, command.insert_point), ToPagePoint(page_height, command.insert_point));
  paragraph_editing.SetFontSize(command.font_size);
  paragraph_editing.SetUnderline(command.underline);
  paragraph_editing.InsertText((const wchar_t*)command.insert_text);
  paragraph_editing.OnLButtonDown(0, ToPagePoint(page_height, command.select_start));
  paragraph_editing.OnMouseMove(0, ToPagePoint(page_height, command.select_end));
  paragraph_editing.OnLButtonUp(0, ToPagePoint(page_height, command.select_end));
  paragraph_editing.DeleteSelected();
  paragraph_editing.Deactivate();
  return true;
}

bool ExecuteSplitOperation(JoinSplit& join_split, float page_height, const ParagraphEditingCommand& command) {
  join_split.Activate();
  join_split.OnLButtonDown(0, ToPagePoint(page_height, command.split_point));
  join_split.OnLButtonUp(0, ToPagePoint(page_height, command.split_point));
  join_split.SplitBoxes();
  join_split.Deactivate();
  return true;
}

bool ExecuteJoinOperation(JoinSplit& join_split, float page_height, const ParagraphEditingCommand& command) {
  if (command.join_points.size() < 2) {
    printf("At least 2 join points are required.\n");
    return false;
  }
  join_split.Activate();
  for (size_t i = 0; i < command.join_points.size(); ++i) {
    join_split.OnLButtonDown(0, ToPagePoint(page_height, command.join_points[i]));
    join_split.OnLButtonUp(0, ToPagePoint(page_height, command.join_points[i]));
  }
  join_split.JoinBoxes();
  join_split.Deactivate();
  return true;
}

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

class FxParagraphEditingProviderCallback : public foxit::addon::pageeditor::ParagraphEditingProviderCallback {
 public:
  FxParagraphEditingProviderCallback(int page_index) { this->current_page_index_ = page_index; }
  virtual ~FxParagraphEditingProviderCallback() {}

  virtual void Release() { delete this; }

  virtual foxit::Matrix GetRenderMatrix(const pdf::PDFDoc& document, int page_index) {
    PDFPage page = (PDFDoc(document)).GetPage(page_index);
    int width = static_cast<int>(page.GetWidth());
    int height = static_cast<int>(page.GetHeight());
    Matrix matrix = page.GetDisplayMatrix(0, 0, width, height, e_Rotation0);
    return matrix;
  }
  virtual void* GetPageViewHandle(const pdf::PDFDoc& document, int page_index) { return NULL; }
  virtual foxit::RectF GetClientRect(const pdf::PDFDoc& document) { return foxit::RectF(); }
  virtual float GetScale(const pdf::PDFDoc& document, int page_index) { return 1.0f; }
  virtual bool GotoPageView(const pdf::PDFDoc& document, int page_index, float left, float top) { return true; }
  virtual Int32Array GetVisiblePageIndexArray(const pdf::PDFDoc& document) {
    Int32Array page_array;
    int pageindex = this->current_page_index_;
    page_array.Add(pageindex);
    return page_array;
  }
  virtual RectF GetPageVisibleRect(const pdf::PDFDoc& document, int page_index) { return foxit::RectF(); }
  virtual foxit::RectF GetPageRect(const pdf::PDFDoc& document, int page_index) {
    PDFDoc doc = document;
    PDFPage page = doc.GetPage(page_index);
    float width = page.GetWidth();
    float height = page.GetHeight();
    RectF rect;
    rect.left = 0;
    rect.bottom = height;
    rect.right = width;
    rect.top = 0;
    return rect;
  }
  virtual int GetCurrentPageIndex(const pdf::PDFDoc& document) { return this->current_page_index_; }
  virtual common::Rotation GetRotation(const pdf::PDFDoc& document, int page_index) {
    PDFPage page = (PDFDoc(document)).GetPage(page_index);
    Rotation rotation = page.GetRotation();
    return rotation;
  }
  virtual void InvalidateRect(const pdf::PDFDoc& document, int page_index, const RectFArray& invalid_rects) {}
  virtual void AddUndoItem(const ParagraphEditingUndoItem& undo_item) {}
  virtual void SetDocChangeMark(const pdf::PDFDoc& document) {}
  virtual void NotifyTextInputReachLimit(const pdf::PDFDoc& document, int page_index) {}

 private:
  int current_page_index_;
};

int main(int argc, char* argv[]) {
  ParagraphEditingCommand command;
  if (!ParseCommand(argc, argv, command)) {
    return command.show_help ? 0 : 1;
  }
  EnsureOutputDir(command.output_dir);

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    PDFDoc doc(command.input_file);
    ErrorCode code = doc.Load();
    if (code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(command.input_file), error_code);
      return 1;
    }

    PDFPage page = doc.GetPage(command.page_index);
    if (page.IsEmpty()) {
      printf("Invalid page index: %d\n", command.page_index);
      return 1;
    }
    page.StartParse();
    float page_height = page.GetHeight();

    FxParagraphEditingProviderCallback* callback = new FxParagraphEditingProviderCallback(page.GetIndex());
    ParagraphEditingMgr touchup_mgr = ParagraphEditingMgr(callback, doc);
    ParagraphEditing paragraph_editing = touchup_mgr.GetParagraphEditing();
    JoinSplit join_split = touchup_mgr.GetJoinSplit();

    for (size_t i = 0; i < command.ops.size(); ++i) {
      const string& op = command.ops[i];
      if (op == "paragraph") {
        if (!ExecuteParagraphOperation(paragraph_editing, page_height, command)) return 1;
        if (!SaveStepOutput(doc, command.output_dir, command.save_prefix, L"_paragraph")) return 1;
      } else if (op == "split") {
        if (!ExecuteSplitOperation(join_split, page_height, command)) return 1;
        if (!SaveStepOutput(doc, command.output_dir, command.save_prefix, L"_split")) return 1;
      } else if (op == "join") {
        if (!ExecuteJoinOperation(join_split, page_height, command)) return 1;
        if (!SaveStepOutput(doc, command.output_dir, command.save_prefix, L"_join")) return 1;
      }
    }
    cout << "Paragraph editing successfully." << endl;
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  } catch (...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}
