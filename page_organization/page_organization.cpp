// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to do PDF page organization,
// such as inserting, removing, and so on.

// Include Foxit SDK header files.
#include <time.h>
#include <iostream>
#include <algorithm>
#include <cerrno>
#include <climits>
#include <cctype>
#include <cstdlib>
#include <set>
#include <vector>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/graphics/fs_pdfgraphicsobject.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace graphics;

enum OperationType {
  kOperationInsert,
  kOperationRemove,
  kOperationImport,
  kOperationMove
};

struct InsertOptions {
  String target;
  WString text;
  int at_index;
  bool has_index;
  InsertOptions() : target("current"), text(L""), at_index(-1), has_index(false) {}
};

struct RemoveOptions {
  String target;
  String mode;
  int index;
  bool has_index;
  RemoveOptions() : target("current"), mode(""), index(-1), has_index(false) {}
};

struct ImportOptions {
  String target;
  String source;
  String mode;
  ImportOptions() : target("current"), source(""), mode("normal") {}
};

struct MoveOptions {
  String target;
  String mode;
  MoveOptions() : target("current"), mode("") {}
};

struct CliOptions {
  bool show_help;
  WString input_path;
  WString output_file;
  WString source_123_name;
  WString source_abc_name;
  String ops_raw;
  String start_doc;
  InsertOptions insert;
  RemoveOptions remove;
  ImportOptions import;
  MoveOptions move;
  vector<OperationType> operations;
  CliOptions()
      : show_help(false),
        input_path(L""),
        output_file(L""),
        source_123_name(L"page_organization_123.pdf"),
        source_abc_name(L"page_organization_abc.pdf"),
        start_doc("123") {}
};

static const char* sn = "";
static const char* key = "";

#if defined(_WIN32) || defined(_WIN64)
static WString input_path = WString::FromLocal("../input_files/");
#else
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

String ToLowerAscii(const String& value) {
  string text = string((FX_LPCSTR)value, value.GetLength());
  transform(text.begin(), text.end(), text.begin(),
            [](unsigned char ch) { return (char)tolower(ch); });
  return String(text.c_str());
}

bool ParseIntValue(const String& text, int& value) {
  const char* raw = text;
  if (raw == NULL || *raw == '\0') {
    return false;
  }
  char* end_ptr = NULL;
  errno = 0;
  long parsed = strtol(raw, &end_ptr, 10);
  if (errno != 0 || end_ptr == raw || *end_ptr != '\0') {
    return false;
  }
  if (parsed < INT_MIN || parsed > INT_MAX) {
    return false;
  }
  value = (int)parsed;
  return true;
}

vector<String> SplitCsv(const String& raw) {
  vector<String> values;
  string text = string((FX_LPCSTR)raw, raw.GetLength());
  size_t start = 0;
  while (start <= text.size()) {
    size_t comma = text.find(',', start);
    size_t end = (comma == string::npos) ? text.size() : comma;
    string token = text.substr(start, end - start);
    size_t left = 0;
    while (left < token.size() && isspace((unsigned char)token[left])) {
      ++left;
    }
    size_t right = token.size();
    while (right > left && isspace((unsigned char)token[right - 1])) {
      --right;
    }
    if (right > left) {
      values.push_back(String(token.substr(left, right - left).c_str()));
    }
    if (comma == string::npos) {
      break;
    }
    start = comma + 1;
  }
  return values;
}

WString EnsureTrailingSlash(const WString& directory) {
  if (directory.IsEmpty()) {
    return directory;
  }
  size_t length = directory.GetLength();
  wchar_t last_char = directory[length - 1];
  if (last_char == L'/' || last_char == L'\\') {
    return directory;
  }
  return directory + L"/";
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
  if (directory.IsEmpty()) {
    return;
  }
#if defined(_WIN32) || defined(_WIN64)
  _mkdir(String::FromUnicode(directory));
#else
  mkdir(String::FromUnicode(directory), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

void PrintUsage() {
  cout << "Usage:" << endl
       << "  page_organization --input-dir <dir> --output <pdf> --ops <op1,op2,...> [options]" << endl
       << endl
       << "Required:" << endl
       << "  --input <pdf>            Input PDF file path" << endl
       << "  --output <pdf>            Final output PDF path" << endl
       << "  --ops <list>              Operation sequence, items from: insert,remove,import,move" << endl
       << endl
       << "Common optional:" << endl
       << "Insert options (when ops contains insert):" << endl
       << "  --insert-text <text>                Text written to the inserted page (required)" << endl
       << "  --insert-at <index>                 Insert index, -1 for append (default: -1)" << endl
       << endl
       << "Remove options (when ops contains remove):" << endl
       << "  --remove-target <current|123|abc>   Target doc (default: current)" << endl
       << "  --remove-mode <all-except-last|single>" << endl
       << "  --remove-index <index>              Required when mode=single" << endl
       << endl
       << "Import options (when ops contains import):" << endl
       << "  --import-target <current|123|abc>   Target doc (default: current)" << endl
       << "  --import-src <123|abc>              Source doc key (required)" << endl
       << "  --import-mode <normal|optimized>    Import mode (default: normal)" << endl
       << endl
       << "Move options (when ops contains move):" << endl
       << "  --move-target <current|123|abc>     Target doc (default: current)" << endl
       << "  --move-mode <first-to-last|even-to-last> (required)" << endl
       << endl
       << "Help:" << endl
       << "  --help                              Show this help message" << endl;
}

bool IsTokenIn(const String& token, const char* a, const char* b, const char* c = NULL) {
  if (token.Equal(a) || token.Equal(b)) {
    return true;
  }
  if (c != NULL && token.Equal(c)) {
    return true;
  }
  return false;
}

bool ParseArgs(int argc, char* argv[], CliOptions& options, String& error_message) {
  for (int i = 1; i < argc; ++i) {
    String key = ToLowerAscii(String(argv[i]));
    if (key.Equal("--help") || key.Equal("-h")) {
      options.show_help = true;
      return true;
    }
    if (i + 1 >= argc) {
      error_message = "Missing value for argument: " + key;
      return false;
    }

    String value = String(argv[++i]);
    if (key.Equal("--input") || key.Equal("-i")) {
      options.input_path = WString::FromUTF8(value);
    } else if (key.Equal("--output") || key.Equal("-o")) {
      options.output_file = WString::FromUTF8(value);
    } else if (key.Equal("--ops")) {
      options.ops_raw = value;
    } else if (key.Equal("--start-doc")) {
      options.start_doc = ToLowerAscii(value);
    } else if (key.Equal("--file-123")) {
      options.source_123_name = WString::FromUTF8(value);
    } else if (key.Equal("--file-abc")) {
      options.source_abc_name = WString::FromUTF8(value);
    } else if (key.Equal("--insert-target")) {
      options.insert.target = ToLowerAscii(value);
    } else if (key.Equal("--insert-text")) {
      options.insert.text = WString::FromUTF8(value);
    } else if (key.Equal("--insert-at")) {
      if (!ParseIntValue(value, options.insert.at_index)) {
        error_message = "Invalid value for --insert-at: " + value;
        return false;
      }
      options.insert.has_index = true;
    } else if (key.Equal("--remove-target")) {
      options.remove.target = ToLowerAscii(value);
    } else if (key.Equal("--remove-mode")) {
      options.remove.mode = ToLowerAscii(value);
    } else if (key.Equal("--remove-index")) {
      if (!ParseIntValue(value, options.remove.index)) {
        error_message = "Invalid value for --remove-index: " + value;
        return false;
      }
      options.remove.has_index = true;
    } else if (key.Equal("--import-target")) {
      options.import.target = ToLowerAscii(value);
    } else if (key.Equal("--import-src")) {
      options.import.source = ToLowerAscii(value);
    } else if (key.Equal("--import-mode")) {
      options.import.mode = ToLowerAscii(value);
    } else if (key.Equal("--move-target")) {
      options.move.target = ToLowerAscii(value);
    } else if (key.Equal("--move-mode")) {
      options.move.mode = ToLowerAscii(value);
    } else {
      error_message = "Unknown argument: " + key;
      return false;
    }
  }
  return true;
}

bool ParseOperations(CliOptions& options, String& error_message) {
  vector<String> tokens = SplitCsv(options.ops_raw);
  if (tokens.empty()) {
    error_message = "--ops cannot be empty.";
    return false;
  }

  set<string> appeared;
  for (size_t i = 0; i < tokens.size(); ++i) {
    String op = ToLowerAscii(tokens[i]);
    if (op.IsEmpty()) {
      continue;
    }
    string op_text = string((FX_LPCSTR)op, op.GetLength());
    if (appeared.find(op_text) != appeared.end()) {
      error_message = "Duplicate operation in --ops is not supported: " + op;
      return false;
    }
    appeared.insert(op_text);

    if (op.Equal("insert")) {
      options.operations.push_back(kOperationInsert);
    } else if (op.Equal("remove")) {
      options.operations.push_back(kOperationRemove);
    } else if (op.Equal("import")) {
      options.operations.push_back(kOperationImport);
    } else if (op.Equal("move")) {
      options.operations.push_back(kOperationMove);
    } else {
      error_message = "Unsupported operation in --ops: " + op;
      return false;
    }
  }

  return !options.operations.empty();
}

bool ValidateDocToken(const String& token) {
  return token.Equal("current") || token.Equal("123") || token.Equal("abc");
}

bool PathExists(const WString& path) {
#if defined(_WIN32) || defined(_WIN64)
  struct _stat info;
  return _stat(String::FromUnicode(path), &info) == 0;
#else
  struct stat info;
  return stat(String::FromUnicode(path), &info) == 0;
#endif
}

bool ValidateArgs(CliOptions& options, String& error_message) {
  if (options.show_help) {
    return true;
  }

  if (options.input_path.IsEmpty()) {
    error_message = "Missing required argument: --input-dir";
    return false;
  }
  if (options.output_file.IsEmpty()) {
    error_message = "Missing required argument: --output";
    return false;
  }
  if (!PathExists(options.input_path)) {
    error_message = "Input directory does not exist: " + String::FromUnicode(options.input_path);
    return false;
  }
  if (!options.start_doc.Equal("123") && !options.start_doc.Equal("abc")) {
    error_message = "Invalid --start-doc, expected 123|abc";
    return false;
  }
  if (!ParseOperations(options, error_message)) {
    return false;
  }

  bool has_insert = false;
  bool has_remove = false;
  bool has_import = false;
  bool has_move = false;
  for (size_t i = 0; i < options.operations.size(); ++i) {
    switch (options.operations[i]) {
      case kOperationInsert: has_insert = true; break;
      case kOperationRemove: has_remove = true; break;
      case kOperationImport: has_import = true; break;
      case kOperationMove: has_move = true; break;
    }
  }

  if (has_insert) {
    if (!ValidateDocToken(options.insert.target)) {
      error_message = "Invalid --insert-target, expected current|123|abc";
      return false;
    }
    if (options.insert.text.IsEmpty()) {
      error_message = "Missing required argument for insert: --insert-text";
      return false;
    }
  }

  if (has_remove) {
    if (!ValidateDocToken(options.remove.target)) {
      error_message = "Invalid --remove-target, expected current|123|abc";
      return false;
    }
    if (!options.remove.mode.Equal("all-except-last") && !options.remove.mode.Equal("single")) {
      error_message = "Invalid --remove-mode, expected all-except-last|single";
      return false;
    }
    if (options.remove.mode.Equal("single") && !options.remove.has_index) {
      error_message = "--remove-index is required when --remove-mode=single";
      return false;
    }
  }

  if (has_import) {
    if (!ValidateDocToken(options.import.target)) {
      error_message = "Invalid --import-target, expected current|123|abc";
      return false;
    }
    if (!options.import.source.Equal("123") && !options.import.source.Equal("abc")) {
      error_message = "Invalid --import-src, expected 123|abc";
      return false;
    }
    if (!options.import.mode.Equal("normal") && !options.import.mode.Equal("optimized")) {
      error_message = "Invalid --import-mode, expected normal|optimized";
      return false;
    }
  }

  if (has_move) {
    if (!ValidateDocToken(options.move.target)) {
      error_message = "Invalid --move-target, expected current|123|abc";
      return false;
    }
    if (!options.move.mode.Equal("first-to-last") && !options.move.mode.Equal("even-to-last")) {
      error_message = "Invalid --move-mode, expected first-to-last|even-to-last";
      return false;
    }
  }

  return true;
}

bool LoadDocument(const WString& file_path, PDFDoc& doc) {
  doc = PDFDoc(file_path);
  ErrorCode code = doc.Load();
  if (code != foxit::e_ErrSuccess) {
    printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(file_path), code);
    return false;
  }
  return true;
}

bool ResolveTargetDocument(const CliOptions& options, const String& target, PDFDoc& current_doc, bool& has_current) {
  if (target.Equal("current")) {
    if (!has_current) {
      cout << "No current document available. Please set --start-doc or use target 123/abc first." << endl;
      return false;
    }
    return true;
  }
  WString file_path = options.input_path;
  if (!LoadDocument(file_path, current_doc)) {
    return false;
  }
  has_current = true;
  return true;
}

void AddTextObjects(PDFPage page, const wchar_t* content) {
  POSITION position = page.GetLastGraphicsObjectPosition(GraphicsObject::e_TypeText);
  TextObject* text_object = TextObject::Create();

  text_object->SetFillColor(0xFFAAAAAA);
  text_object->SetStrokeColor(0xFFF68C21);

  // Prepare text state
  TextState state;
  state.font_size = 64.0f;
  state.font = Font(Font::e_StdIDTimes);
  state.textmode = TextState::e_ModeFillStrokeClip;

  text_object->SetTextState(page, state, false, 750);

  // Set content
  text_object->SetText(content);
  page.InsertGraphicsObject(position, text_object);

  // Transform to center
  RectF rect = text_object->GetRect();
  float offset_x = (page.GetWidth() - (rect.right - rect.left)) / 2;
  float offset_y = (page.GetHeight() - (rect.top - rect.bottom)) / 2;
  text_object->Transform(Matrix(1, 0, 0, 1, offset_x, offset_y), false);

  // Generator content
  page.GenerateContent();
}

bool ExecuteInsert(PDFDoc& doc, const InsertOptions& options) {
  int index = options.has_index ? options.at_index : doc.GetPageCount();
  if (index < -1 || index > doc.GetPageCount()) {
    cout << "Invalid insert index: " << index << endl;
    return false;
  }

  if (index == -1) {
    index = doc.GetPageCount();
  }
  PDFPage page = doc.InsertPage(index);
  AddTextObjects(page, options.text);
  cout << "Operation insert completed." << endl;
  return true;
}

bool ExecuteRemove(PDFDoc& doc, const RemoveOptions& options) {
  if (options.mode.Equal("all-except-last")) {
    while (doc.GetPageCount() > 1) {
      doc.RemovePage(0);
    }
  } else {
    if (options.index < 0 || options.index >= doc.GetPageCount()) {
      cout << "Invalid --remove-index: " << options.index << endl;
      return false;
    }
    doc.RemovePage(options.index);
  }
  cout << "Operation remove completed." << endl;
  return true;
}

bool ExecuteImport(PDFDoc& doc_dest, const CliOptions& cli_options, const ImportOptions& options) {
  WString file_src = cli_options.input_path;
  pdf::PDFDoc doc_src = pdf::PDFDoc((FS_HANDLE)NULL);
  if (!LoadDocument(file_src, doc_src)) {
    return false;
  }
  if (doc_src.GetPageCount() <= 0) {
    cout << "Import source document has no pages." << endl;
    return false;
  }

  Range import_ranges(0);
  import_ranges.AddSingle(doc_src.GetPageCount() - 1);

  Progressive progressive;
  if (options.mode.Equal("optimized")) {
    WString temp_file = ParentDirectory(cli_options.output_file) + L"page_organization_import_temp.pdf";
    progressive = doc_src.StartExtractPages(temp_file, PDFDoc::e_ExtractPagesOptionAnnotation, import_ranges, NULL);
    while (progressive.Continue() != Progressive::e_Finished) {
    }

    PDFDoc doc_temp;
    if (!LoadDocument(temp_file, doc_temp)) {
      return false;
    }
    doc_dest.InsertDocument(-1, doc_temp, PDFDoc::e_InsertDocOptionAttachments);
  } else {
    progressive = doc_dest.StartImportPages(-1, doc_src, PDFDoc::e_ImportFlagNormal, "abc", import_ranges, NULL);
    while (progressive.Continue() != Progressive::e_Finished) {
    }
  }

  cout << "Operation import completed." << endl;
  return true;
}

bool ExecuteMove(PDFDoc& doc, const MoveOptions& options) {
  int count = doc.GetPageCount();
  if (count <= 0) {
    cout << "Move target document has no pages." << endl;
    return false;
  }

  if (options.mode.Equal("first-to-last")) {
    PDFPage page = doc.GetPage(0);
    doc.MovePageTo(page, doc.GetPageCount() - 1);
  } else {
    Range page_set;
    for (int i = 0; i < count / 2; i++) {
      page_set.AddSingle(2 * i);
    }
    doc.MovePagesTo(page_set, doc.GetPageCount() - 1);
  }
  cout << "Operation move completed." << endl;
  return true;
}

bool ExecuteOperations(const CliOptions& options, PDFDoc& current_doc) {
  bool has_current = false;
  if (!ResolveTargetDocument(options, options.start_doc, current_doc, has_current)) {
    return false;
  }

  for (size_t i = 0; i < options.operations.size(); ++i) {
    OperationType op = options.operations[i];
    bool ok = false;
    switch (op) {
      case kOperationInsert:
        if (!ResolveTargetDocument(options, options.insert.target, current_doc, has_current)) {
          return false;
        }
        ok = ExecuteInsert(current_doc, options.insert);
        break;
      case kOperationRemove:
        if (!ResolveTargetDocument(options, options.remove.target, current_doc, has_current)) {
          return false;
        }
        ok = ExecuteRemove(current_doc, options.remove);
        break;
      case kOperationImport:
        if (!ResolveTargetDocument(options, options.import.target, current_doc, has_current)) {
          return false;
        }
        ok = ExecuteImport(current_doc, options, options.import);
        break;
      case kOperationMove:
        if (!ResolveTargetDocument(options, options.move.target, current_doc, has_current)) {
          return false;
        }
        ok = ExecuteMove(current_doc, options.move);
        break;
      default:
        ok = false;
        break;
    }

    if (!ok) {
      cout << "Operation failed at step " << (int)(i + 1) << "." << endl;
      return false;
    }
  }

  if (!has_current) {
    cout << "No operation produced a working document." << endl;
    return false;
  }
  return true;
}

int main(int argc, char *argv[])
{
  CliOptions options;
  String error_message;

  if (!ParseArgs(argc, argv, options, error_message)) {
    cout << error_message << endl;
    PrintUsage();
    return 1;
  }

  if (options.show_help) {
    PrintUsage();
    return 0;
  }

  if (!ValidateArgs(options, error_message)) {
    cout << error_message << endl;
    PrintUsage();
    return 1;
  }

  EnsureDirectoryExists(ParentDirectory(options.output_file));

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    PDFDoc current_doc = PDFDoc((FS_HANDLE)NULL);
    if (!ExecuteOperations(options, current_doc)) {
      return 1;
    }

    current_doc.SaveAs(options.output_file, PDFDoc::e_SaveFlagNormal);
    cout << "All requested operations completed. Output: "
         << (const char*)String::FromUnicode(options.output_file) << endl;
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  }
  catch(...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}
