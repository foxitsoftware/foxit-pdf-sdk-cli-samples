// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to combine pdf files.
// Include Foxit SDK header files.
#include <iostream>
#include <string>
#include <vector>
#include <cctype>

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_combination.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

struct InputEntry {
  WString file_path;
  WString bookmark_title;
};

struct PdfCombinationCommand {
  vector<InputEntry> inputs;
  WString output_file;

  // Options flags (correspond to Combination::CombineDocsOptions)
  bool opt_bookmark;           // e_CombineDocsOptionBookmark       (0x0001)
  bool opt_acroform_rename;    // e_CombineDocsOptionAcroformRename (0x0002)
  bool opt_structure_tree;     // e_CombineDocsOptionStructrueTree  (0x0004)
  bool opt_output_intents;     // e_CombineDocsOptionOutputIntents  (0x0008)
  bool opt_oc_properties;      // e_CombineDocsOptionOCProperties   (0x0010)
  bool opt_mark_infos;         // e_CombineDocsOptionMarkInfos      (0x0020)
  bool opt_page_labels;        // e_CombineDocsOptionPageLabels     (0x0040)
  bool opt_names;              // e_CombineDocsOptionNames          (0x0080)
  bool opt_object_stream;      // e_CombineDocsOptionObjectStream   (0x0100)
  bool opt_duplicate_stream;   // e_CombineDocsOptionDuplicateStream(0x0200)

  bool show_help;

  PdfCombinationCommand()
      : opt_bookmark(false),
        opt_acroform_rename(false),
        opt_structure_tree(true),
        opt_output_intents(true),
        opt_oc_properties(true),
        opt_mark_infos(true),
        opt_page_labels(true),
        opt_names(true),
        opt_object_stream(true),
        opt_duplicate_stream(true),
        show_help(false) {}
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
  cout << "pdfcombination --input <file.pdf> [--bookmark <title>] ... --output <combined.pdf> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --input <path>                  Input PDF path. Can be specified multiple times." << endl;
  cout << "  --output <path>                 Output combined PDF file path." << endl << endl;
  cout << "Optional:" << endl;
  cout << "  --bookmark <title>              Bookmark title for the preceding --input file." << endl;
  cout << "                                  If omitted, the input filename is used." << endl;
  cout << "  --opt-bookmark <true|false>     Output bookmarks to combined PDF (default: false)." << endl;
  cout << "  --opt-acroform-rename <true|false>  Rename duplicate form fields (default: false)." << endl;
  cout << "  --opt-structure-tree <true|false>   Output structure trees (default: true)." << endl;
  cout << "  --opt-output-intents <true|false>   Output output intents (default: true)." << endl;
  cout << "  --opt-oc-properties <true|false>    Output OCProperties/layers (default: true)." << endl;
  cout << "  --opt-mark-infos <true|false>       Output MarkInfo (default: true)." << endl;
  cout << "  --opt-page-labels <true|false>      Output page labels (default: true)." << endl;
  cout << "  --opt-names <true|false>            Output Dests/EmbeddedFiles name trees (default: true)." << endl;
  cout << "  --opt-object-stream <true|false>    Use object streams to reduce file size (default: true)." << endl;
  cout << "  --opt-duplicate-stream <true|false> Output duplicate stream objects (default: true)." << endl;
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

// Extract filename from a full path (without extension).
WString ExtractFileName(const WString& file_path) {
  if (file_path.IsEmpty()) return WString();
  // Find last separator.
  int last_sep = -1;
  const wchar_t* str = (const wchar_t*)file_path;
  int len = file_path.GetLength();
  for (int i = len - 1; i >= 0; --i) {
    if (str[i] == L'\\' || str[i] == L'/') {
      last_sep = i;
      break;
    }
  }
  WString name;
  if (last_sep >= 0) {
    name = file_path.Mid(last_sep + 1, len - last_sep - 1);
  } else {
    name = file_path;
  }
  // Remove extension if present.
  int dot_pos = -1;
  const wchar_t* name_str = (const wchar_t*)name;
  int name_len = name.GetLength();
  for (int i = name_len - 1; i >= 0; --i) {
    if (name_str[i] == L'.') {
      dot_pos = i;
      break;
    }
  }
  if (dot_pos > 0) {
    return name.Mid(0, dot_pos);
  }
  return name;
}

bool ParseBoolValue(const String& value, bool& out_value) {
  string text = (const char*)value;
  for (size_t i = 0; i < text.size(); ++i)
    text[i] = (char)tolower((unsigned char)text[i]);
  if (text == "true" || text == "1" || text == "yes") { out_value = true; return true; }
  if (text == "false" || text == "0" || text == "no") { out_value = false; return true; }
  return false;
}

bool ParseCommand(int argc, char* argv[], PdfCombinationCommand& command) {
  if (argc <= 1) {
    PrintUsage();
    return false;
  }

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
    if (key.Equal("--input") || key.Equal("-i")) {
      InputEntry entry;
      entry.file_path = WString::FromUTF8(value);
      entry.bookmark_title = WString();
      command.inputs.push_back(entry);
    } else if (key.Equal("--bookmark") || key.Equal("-b")) {
      if (command.inputs.empty()) {
        printf("--bookmark must follow an --input option.\n");
        return false;
      }
      command.inputs.back().bookmark_title = WString::FromUTF8(value);
    } else if (key.Equal("--output") || key.Equal("-o")) {
      command.output_file = WString::FromUTF8(value);
      has_output = true;
    } else if (key.Equal("--opt-bookmark")) {
      if (!ParseBoolValue(value, command.opt_bookmark)) {
        printf("Invalid value for --opt-bookmark: %s\n", (const char*)value); return false;
      }
    } else if (key.Equal("--opt-acroform-rename")) {
      if (!ParseBoolValue(value, command.opt_acroform_rename)) {
        printf("Invalid value for --opt-acroform-rename: %s\n", (const char*)value); return false;
      }
    } else if (key.Equal("--opt-structure-tree")) {
      if (!ParseBoolValue(value, command.opt_structure_tree)) {
        printf("Invalid value for --opt-structure-tree: %s\n", (const char*)value); return false;
      }
    } else if (key.Equal("--opt-output-intents")) {
      if (!ParseBoolValue(value, command.opt_output_intents)) {
        printf("Invalid value for --opt-output-intents: %s\n", (const char*)value); return false;
      }
    } else if (key.Equal("--opt-oc-properties")) {
      if (!ParseBoolValue(value, command.opt_oc_properties)) {
        printf("Invalid value for --opt-oc-properties: %s\n", (const char*)value); return false;
      }
    } else if (key.Equal("--opt-mark-infos")) {
      if (!ParseBoolValue(value, command.opt_mark_infos)) {
        printf("Invalid value for --opt-mark-infos: %s\n", (const char*)value); return false;
      }
    } else if (key.Equal("--opt-page-labels")) {
      if (!ParseBoolValue(value, command.opt_page_labels)) {
        printf("Invalid value for --opt-page-labels: %s\n", (const char*)value); return false;
      }
    } else if (key.Equal("--opt-names")) {
      if (!ParseBoolValue(value, command.opt_names)) {
        printf("Invalid value for --opt-names: %s\n", (const char*)value); return false;
      }
    } else if (key.Equal("--opt-object-stream")) {
      if (!ParseBoolValue(value, command.opt_object_stream)) {
        printf("Invalid value for --opt-object-stream: %s\n", (const char*)value); return false;
      }
    } else if (key.Equal("--opt-duplicate-stream")) {
      if (!ParseBoolValue(value, command.opt_duplicate_stream)) {
        printf("Invalid value for --opt-duplicate-stream: %s\n", (const char*)value); return false;
      }
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (command.inputs.empty()) {
    printf("At least one --input is required.\n");
    PrintUsage();
    return false;
  }
  if (!has_output) {
    printf("--output is required.\n");
    PrintUsage();
    return false;
  }
  // Validate input files exist.
  for (size_t idx = 0; idx < command.inputs.size(); ++idx) {
    if (!FileExists(command.inputs[idx].file_path)) {
      printf("Input file does not exist: %s\n",
             (const char*)String::FromUnicode(command.inputs[idx].file_path));
      return false;
    }
  }
  return true;
}

int main(int argc, char* argv[]) {
  PdfCombinationCommand command;
  if (!ParseCommand(argc, argv, command)) {
    return 1;
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    cout << "===Start combine pdf files===" << endl;

    CombineDocumentInfoArray combine_document_array;
    for (size_t i = 0; i < command.inputs.size(); ++i) {
      const InputEntry& entry = command.inputs[i];
      CombineDocumentInfo file_info(entry.file_path, L"");
      // Set bookmark title: use provided title or derive from filename.
      WString bookmark = entry.bookmark_title.IsEmpty()
                             ? ExtractFileName(entry.file_path)
                             : entry.bookmark_title;
      file_info.SetBookmarkTitle(bookmark);
      // Set PDF file name: use the base filename (with extension).
      WString pdf_filename = ExtractFileName(entry.file_path);
      // Re-attach extension for PDFFileName.
      const wchar_t* fp = (const wchar_t*)entry.file_path;
      int fp_len = entry.file_path.GetLength();
      bool has_pdf_ext = (fp_len > 4 &&
                          (fp[fp_len - 4] == L'.') &&
                          (fp[fp_len - 3] == L'p' || fp[fp_len - 3] == L'P') &&
                          (fp[fp_len - 2] == L'd' || fp[fp_len - 2] == L'D') &&
                          (fp[fp_len - 1] == L'f' || fp[fp_len - 1] == L'F'));
      if (has_pdf_ext) {
        // Extract full filename with extension from path.
        int last_sep = -1;
        for (int j = fp_len - 1; j >= 0; --j) {
          if (fp[j] == L'\\' || fp[j] == L'/') {
            last_sep = j;
            break;
          }
        }
        pdf_filename = entry.file_path.Mid(last_sep + 1, fp_len - last_sep - 1);
      }
      file_info.SetPDFFileName(pdf_filename);
      combine_document_array.Add(file_info);
    }

    uint32 options = 0;
    if (command.opt_bookmark)         options |= Combination::e_CombineDocsOptionBookmark;
    if (command.opt_acroform_rename)  options |= Combination::e_CombineDocsOptionAcroformRename;
    if (command.opt_structure_tree)   options |= Combination::e_CombineDocsOptionStructrueTree;
    if (command.opt_output_intents)   options |= Combination::e_CombineDocsOptionOutputIntents;
    if (command.opt_oc_properties)    options |= Combination::e_CombineDocsOptionOCProperties;
    if (command.opt_mark_infos)       options |= Combination::e_CombineDocsOptionMarkInfos;
    if (command.opt_page_labels)      options |= Combination::e_CombineDocsOptionPageLabels;
    if (command.opt_names)            options |= Combination::e_CombineDocsOptionNames;
    if (command.opt_object_stream)    options |= Combination::e_CombineDocsOptionObjectStream;
    if (command.opt_duplicate_stream) options |= Combination::e_CombineDocsOptionDuplicateStream;

    Progressive progressive = Combination::StartCombineDocuments(command.output_file, combine_document_array, options);
    if (progressive.GetRateOfProgress() != 100) {
      Progressive::State state = Progressive::e_ToBeContinued;
      while (Progressive::e_ToBeContinued == state) {
        state = progressive.Continue();
      }
    }
    cout << "===PDF combination end===" << endl;
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  } catch (...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}
