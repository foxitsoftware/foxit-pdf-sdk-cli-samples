// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to get/set page label information
// in a PDF document.

#include <time.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/graphics/fs_pdfgraphicsobject.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/fs_pdfpagelabel.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

enum PageLabelOperation {
  e_OpCreate = 0,
  e_OpDelete,
  e_OpUpdate,
  e_OpQuery
};

struct PageLabelsCommand {
  WString input_file;
  WString output_file;
  WString report_file;
  WString prefix;
  int page_index;
  int start_number;
  PageLabels::Style style;
  PageLabelOperation operation;
  bool remove_all;

  bool has_input;
  bool has_output;
  bool has_op;
  bool has_page_index;
  bool has_style;
  bool has_report_file;

  PageLabelsCommand()
      : page_index(-1),
        start_number(1),
        style(PageLabels::e_DecimalNums),
        operation(e_OpQuery),
        remove_all(false),
        has_input(false),
        has_output(false),
        has_op(false),
        has_page_index(false),
        has_style(false),
        has_report_file(false) {}
};

void PrintUsage() {
  cout << "Usage:" << endl
      << "page_labels --op <create|delete|update|query> --input <input.pdf> [options]" << endl
      << endl
      << "Required:" << endl
      << "--op <create|delete|update|query>" << endl
      << "--input <path>" << endl
      << endl
      << "Common optional:" << endl
      << "--output <path>                  Required for create/delete/update." << endl
      << "--page-index <int>               Required for create/update; optional for query." << endl
      << "--report-file <path>             Query result text output file." << endl
      << endl
      << "Create/Update optional:" << endl
      << "--style <none|decimal|upper-roman|lower-roman|upper-letter|lower-letter>" << endl
      << "--start-number <int>             Default is 1." << endl
      << "--prefix <text>                  Label prefix." << endl
      << endl
      << "Delete optional:" << endl
      << "--remove-all <true|false>        If true, ignore --page-index and delete all labels." << endl;
}

bool ParseIntValue(const String& value, int& out_value) {
  char* end_ptr = NULL;
  out_value = static_cast<int>(strtol((const char*)value, &end_ptr, 10));
  return end_ptr != NULL && *end_ptr == '\0';
}

bool ParseBoolValue(const String& value, bool& out_value) {
  if (value.Equal("true") || value.Equal("1")) {
    out_value = true;
    return true;
  }
  if (value.Equal("false") || value.Equal("0")) {
    out_value = false;
    return true;
  }
  return false;
}

bool ParseOperation(const String& value, PageLabelOperation& op) {
  if (value.Equal("create")) {
    op = e_OpCreate;
    return true;
  }
  if (value.Equal("delete")) {
    op = e_OpDelete;
    return true;
  }
  if (value.Equal("update")) {
    op = e_OpUpdate;
    return true;
  }
  if (value.Equal("query")) {
    op = e_OpQuery;
    return true;
  }
  return false;
}

bool ParseStyle(const String& value, PageLabels::Style& style) {
  if (value.Equal("none")) {
    style = PageLabels::e_None;
    return true;
  }
  if (value.Equal("decimal")) {
    style = PageLabels::e_DecimalNums;
    return true;
  }
  if (value.Equal("upper-roman")) {
    style = PageLabels::e_UpperRomanNums;
    return true;
  }
  if (value.Equal("lower-roman")) {
    style = PageLabels::e_LowerRomanNums;
    return true;
  }
  if (value.Equal("upper-letter")) {
    style = PageLabels::e_UpperLetters;
    return true;
  }
  if (value.Equal("lower-letter")) {
    style = PageLabels::e_LowerLetters;
    return true;
  }
  return false;
}

bool FileExists(const WString& path) {
  FILE* file = NULL;
#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file, String::FromUnicode(path), "rb");
#else
  file = fopen(String::FromUnicode(path), "rb");
#endif
  if (file == NULL) {
    return false;
  }
  fclose(file);
  return true;
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

bool AnalysisParameter(int argc, char* argv[], PageLabelsCommand& command) {
  if (argc < 5 || ((argc - 1) % 2 != 0)) {
    return false;
  }

  for (int i = 1; i < argc; i += 2) {
    String key = String(argv[i]);
    String value = String(argv[i + 1]);
    if (key.Equal("--input")) {
      command.input_file = WString::FromUTF8(value);
      command.has_input = true;
    } else if (key.Equal("--output")) {
      command.output_file = WString::FromUTF8(value);
      command.has_output = true;
    } else if (key.Equal("--op")) {
      if (!ParseOperation(value, command.operation)) {
        return false;
      }
      command.has_op = true;
    } else if (key.Equal("--page-index")) {
      if (!ParseIntValue(value, command.page_index) || command.page_index < 0) {
        return false;
      }
      command.has_page_index = true;
    } else if (key.Equal("--style")) {
      if (!ParseStyle(value, command.style)) {
        return false;
      }
      command.has_style = true;
    } else if (key.Equal("--start-number")) {
      if (!ParseIntValue(value, command.start_number) || command.start_number < 1) {
        return false;
      }
    } else if (key.Equal("--prefix")) {
      command.prefix = WString::FromUTF8(value);
    } else if (key.Equal("--remove-all")) {
      if (!ParseBoolValue(value, command.remove_all)) {
        return false;
      }
    } else if (key.Equal("--report-file")) {
      command.report_file = WString::FromUTF8(value);
      command.has_report_file = true;
    } else {
      return false;
    }
  }

  if (!command.has_input || !command.has_op) {
    return false;
  }
  if (command.operation != e_OpQuery && !command.has_output) {
    return false;
  }
  if ((command.operation == e_OpCreate || command.operation == e_OpUpdate)
      && (!command.has_page_index || !command.has_style)) {
    return false;
  }
  if (command.operation == e_OpDelete && !command.remove_all && !command.has_page_index) {
    return false;
  }

  return true;
}

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

String StyleCodeToString(PageLabels::Style style) {
  switch (style) {
  case PageLabels::e_None: {
    return "None";
               }
  case PageLabels::e_DecimalNums: {
    return "DecimalNums";
                  }
  case PageLabels::e_UpperRomanNums: {
    return "UpperRomanNums";
                     }
  case PageLabels::e_LowerRomanNums: {
    return "LowerRomanNums";
                     }
  case PageLabels::e_UpperLetters: {
    return "UpperLetters";
                   }
  case PageLabels::e_LowerLetters: {
    return "LowerLetters";
                   }
  }
  return "";
}

string BuildPageLabelsInfo(PDFDoc& doc, int page_index) {
  PageLabels page_labels(doc);
  int count = doc.GetPageCount();
  int begin = 0;
  int end = count;
  if (page_index >= 0) {
    begin = page_index;
    end = page_index + 1;
  }

  string result;
  for (int i = begin; i < end; i++) {
    PageLabels::Style style = page_labels.GetPageLabelStyle(i);
    int start_number = page_labels.GetPageLabelStart(i);
    String line;
    line.Format("page index: %d\tstyle: %s\tstart: %d\ttitle: %s\tprefix: %s\r\n", i,
      (const char*)StyleCodeToString(style),
      start_number,
      (const char*)String::FromUnicode(page_labels.GetPageLabelTitle(i)),
      (const char*)String::FromUnicode(page_labels.GetPageLabelPrefix(i)));
    result += string((const char*)line);
  }
  return result;
}

bool WriteTextFile(const WString& path, const string& content) {
  FILE* file = NULL;
#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file, String::FromUnicode(path), "wb");
#else
  file = fopen(String::FromUnicode(path), "wb");
#endif
  if (file == NULL) {
    return false;
  }
  fwrite(content.c_str(), sizeof(char), content.size(), file);
  fclose(file);
  return true;
}

int main(int argc, char *argv[])
{
  if ((argc > 1 && String(argv[1]).Equal("--help")) || argc < 2) {
    PrintUsage();
    return 0;
  }

  PageLabelsCommand command;
  if (!AnalysisParameter(argc, argv, command)) {
    PrintUsage();
    return 1;
  }

  if (!FileExists(command.input_file)) {
    cout << "Input PDF does not exist." << endl;
    return 1;
  }

  if (command.operation != e_OpQuery) {
    if (command.input_file == command.output_file) {
      cout << "Input and output path must be different." << endl;
      return 1;
    }
    EnsureDirectoryExists(ParentDirectory(command.output_file));
  }

  if (command.has_report_file) {
    EnsureDirectoryExists(ParentDirectory(command.report_file));
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    PDFDoc doc(command.input_file);
    ErrorCode code = doc.Load();
    if (code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(command.input_file), code);
      return 1;
    }

    int page_count = doc.GetPageCount();
    if (command.has_page_index && command.page_index >= page_count) {
      cout << "page-index is out of range." << endl;
      return 1;
    }

    PageLabels page_labels(doc);
    if (command.operation == e_OpCreate) {
      if (page_labels.HasPageLabel(command.page_index)) {
        cout << "Target page label already exists; use update operation." << endl;
        return 1;
      }
      page_labels.SetPageLabel(command.page_index, command.style, command.start_number, command.prefix);
      doc.SaveAs(command.output_file, PDFDoc::e_SaveFlagNoOriginal);
      cout << "Page label create success." << endl;
    } else if (command.operation == e_OpUpdate) {
      if (!page_labels.HasPageLabel(command.page_index)) {
        cout << "Target page label does not exist; use create operation." << endl;
        return 1;
      }
      page_labels.SetPageLabel(command.page_index, command.style, command.start_number, command.prefix);
      doc.SaveAs(command.output_file, PDFDoc::e_SaveFlagNoOriginal);
      cout << "Page label update success." << endl;
    } else if (command.operation == e_OpDelete) {
      if (command.remove_all) {
        page_labels.RemoveAll();
      } else {
        if (!page_labels.HasPageLabel(command.page_index)) {
          cout << "Target page label does not exist." << endl;
          return 1;
        }
        page_labels.RemovePageLabel(command.page_index);
      }
      doc.SaveAs(command.output_file, PDFDoc::e_SaveFlagNoOriginal);
      cout << "Page label delete success." << endl;
    } else {
      string report = BuildPageLabelsInfo(doc, command.has_page_index ? command.page_index : -1);
      if (command.has_report_file) {
        if (!WriteTextFile(command.report_file, report)) {
          cout << "Failed to write report file." << endl;
          return 1;
        }
      } else {
        cout << report;
      }
      cout << "Page label query success." << endl;
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

