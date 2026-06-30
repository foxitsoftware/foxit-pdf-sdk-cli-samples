// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to import annotations from FDF/XFDF files
// and export annotations to FDF/XFDF files.

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/fdf/fs_fdfdoc.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/annots/fs_annot.h"
#include "../../../include/pdf/interform/fs_pdfform.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace annots;
using namespace actions;

struct CliOptions {
  WString input_file;
  WString output_file;
  WString fdf_data_file;
  String mode;
  String type;
  bool show_help;

  CliOptions() : show_help(false) {}
};

void PrintUsage() {
  cout << "Usage:" << endl
       << "Export: fdf -i <input.pdf> -o <output.(fdf|xfdf)> --mode <pdf2fdf|pdf2xfdf> --type <annot|form|annot,form>" << endl
       << "Import: fdf -i <input.pdf> --fdf-data <data.(fdf|xfdf)> -o <output.pdf> --mode <fdf2pdf|xfdf2pdf> --type <annot|form|annot,form>" << endl << endl
       << "Required options:" << endl
       << "  -i, --input     Input PDF file path." << endl
       << "  -o, --output    Output file path (FDF/XFDF for export, PDF for import)." << endl
       << "  --mode          Mode: pdf2fdf, pdf2xfdf, fdf2pdf, or xfdf2pdf." << endl
       << "  --type          Data type: annot, form, or annot,form." << endl
       << "  --fdf-data      FDF/XFDF data file path (required for import mode)." << endl
       << "  --help          Show this help message." << endl;
}

bool ParseArgs(int argc, char* argv[], CliOptions& options, string& error_message) {
  for (int i = 1; i < argc; ++i) {
    String key = String(argv[i]);
    if (key.Equal("--help")) {
      options.show_help = true;
      return true;
    }

    if (i + 1 >= argc) {
      error_message = "Missing value for argument: " + std::string((const char*)key);
      return false;
    }

    String value = String(argv[++i]);
    if (key.Equal("-i") || key.Equal("--input")) {
      options.input_file = WString::FromUTF8(value);
    } else if (key.Equal("-o") || key.Equal("--output")) {
      options.output_file = WString::FromUTF8(value);
    } else if (key.Equal("--fdf-data")) {
      options.fdf_data_file = WString::FromUTF8(value);
    } else if (key.Equal("--mode")) {
      options.mode = value;
    } else if (key.Equal("--type")) {
      options.type = value;
    } else {
      error_message = "Unknown argument: " + std::string((const char*)key);
      return false;
    }
  }

  return true;
}

std::string ToLowerAscii(const std::string& value) {
  std::string lower = value;
  std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return lower;
}

bool IsImportMode(const String& mode_value) {
  std::string mode = ToLowerAscii(std::string((const char*)mode_value));
  return (mode == "fdf2pdf" || mode == "xfdf2pdf");
}

bool ValidateArgs(const CliOptions& options, string& error_message) {
  if (options.input_file.IsEmpty()) {
    error_message = "Missing required argument: --input";
    return false;
  }
  if (options.output_file.IsEmpty()) {
    error_message = "Missing required argument: --output";
    return false;
  }
  if (options.mode.IsEmpty()) {
    error_message = "Missing required argument: --mode";
    return false;
  }
  if (options.type.IsEmpty()) {
    error_message = "Missing required argument: --type";
    return false;
  }
  if (IsImportMode(options.mode) && options.fdf_data_file.IsEmpty()) {
    error_message = "Missing required argument: --fdf-data (required for import mode)";
    return false;
  }
  return true;
}

std::string TrimAscii(const std::string& value) {
  size_t begin = 0;
  while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin]))) ++begin;
  size_t end = value.size();
  while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) --end;
  return value.substr(begin, end - begin);
}

bool ParseMode(const String& mode_value, fdf::FDFDoc::Type& fdf_type, bool& is_import, std::string& normalized_mode, std::string& error_message) {
  std::string mode = ToLowerAscii(std::string((const char*)mode_value));
  if (mode == "pdf2fdf") {
    fdf_type = fdf::FDFDoc::e_FDF;
    is_import = false;
    normalized_mode = mode;
    return true;
  }
  if (mode == "pdf2xfdf") {
    fdf_type = fdf::FDFDoc::e_XFDF;
    is_import = false;
    normalized_mode = mode;
    return true;
  }
  if (mode == "fdf2pdf") {
    fdf_type = fdf::FDFDoc::e_FDF;
    is_import = true;
    normalized_mode = mode;
    return true;
  }
  if (mode == "xfdf2pdf") {
    fdf_type = fdf::FDFDoc::e_XFDF;
    is_import = true;
    normalized_mode = mode;
    return true;
  }

  error_message = "Invalid --mode value: " + mode + ". Expected: pdf2fdf, pdf2xfdf, fdf2pdf, or xfdf2pdf.";
  return false;
}

bool ParseExportTypes(const String& type_value, int& export_types, std::string& normalized_type, std::string& error_message) {
  std::string type_text = ToLowerAscii(std::string((const char*)type_value));
  std::vector<std::string> tokens;
  size_t start = 0;
  while (start <= type_text.size()) {
    size_t pos = type_text.find(',', start);
    std::string token = (pos == std::string::npos) ? type_text.substr(start) : type_text.substr(start, pos - start);
    token = TrimAscii(token);
    if (!token.empty()) tokens.push_back(token);
    if (pos == std::string::npos) break;
    start = pos + 1;
  }

  bool has_annot = false;
  bool has_form = false;
  for (size_t i = 0; i < tokens.size(); ++i) {
    if (tokens[i] == "annot") {
      has_annot = true;
    } else if (tokens[i] == "form") {
      has_form = true;
    } else {
      error_message = "Invalid --type token: " + tokens[i] + ". Expected: annot, form, or annot,form.";
      return false;
    }
  }

  if (!has_annot && !has_form) {
    error_message = "Invalid --type value: empty token list. Expected: annot, form, or annot,form.";
    return false;
  }

  export_types = 0;
  if (has_annot) export_types |= PDFDoc::e_Annots;
  if (has_form) export_types |= PDFDoc::e_Forms;

  if (has_annot && has_form) {
    normalized_type = "annot,form";
  } else if (has_annot) {
    normalized_type = "annot";
  } else {
    normalized_type = "form";
  }

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
  if (!ValidateArgs(options, error_message)) {
    cout << error_message << endl;
    PrintUsage();
    return 1;
  }

  fdf::FDFDoc::Type fdf_type = fdf::FDFDoc::e_FDF;
  bool is_import = false;
  std::string normalized_mode;
  if (!ParseMode(options.mode, fdf_type, is_import, normalized_mode, error_message)) {
    cout << error_message << endl;
    PrintUsage();
    return 1;
  }

  int data_types = 0;
  std::string normalized_type;
  if (!ParseExportTypes(options.type, data_types, normalized_type, error_message)) {
    cout << error_message << endl;
    PrintUsage();
    return 1;
  }

  if (!FileExists(options.input_file)) {
    cout << "Input file does not exist: " << String::FromUnicode(options.input_file) << endl;
    return 1;
  }
  if (is_import && !FileExists(options.fdf_data_file)) {
    cout << "FDF data file does not exist: " << String::FromUnicode(options.fdf_data_file) << endl;
    return 1;
  }
  if (FileExists(options.output_file)) {
    cout << "Output file already exists, refusing to overwrite: " << String::FromUnicode(options.output_file) << endl;
    return 1;
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library.
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    pdf::PDFDoc pdf_doc(options.input_file);
    ErrorCode error_code = pdf_doc.Load();
    if (error_code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(options.input_file), error_code);
      return 1;
    }

    if (is_import) {
      // Import FDF/XFDF data into PDF
      fdf::FDFDoc fdf_doc(options.fdf_data_file);
      if (!pdf_doc.ImportFromFDF(fdf_doc, data_types)) {
        cout << "ImportFromFDF failed." << endl;
        return 1;
      }

      if (!pdf_doc.SaveAs(options.output_file, PDFDoc::e_SaveFlagNoOriginal)) {
        cout << "SaveAs failed: " << String::FromUnicode(options.output_file) << endl;
        return 1;
      }

      cout << "Import succeeded. mode=" << normalized_mode
           << ", type=" << normalized_type
           << ", output=" << String::FromUnicode(options.output_file) << endl;
    } else {
      // Export PDF data to FDF/XFDF
      fdf::FDFDoc fdf_doc(fdf_type);
      if (!pdf_doc.ExportToFDF(fdf_doc, data_types)) {
        cout << "ExportToFDF failed." << endl;
        return 1;
      }

      if (!fdf_doc.SaveAs(options.output_file)) {
        cout << "SaveAs failed: " << String::FromUnicode(options.output_file) << endl;
        return 1;
      }

      cout << "Export succeeded. mode=" << normalized_mode
           << ", type=" << normalized_type
           << ", output=" << String::FromUnicode(options.output_file) << endl;
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
