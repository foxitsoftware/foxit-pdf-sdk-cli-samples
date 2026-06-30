// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to create or open a portfolio PDF file.

// Include Foxit SDK header files.
#include <iostream>
#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_filespec.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_portfolio.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace portfolio;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

struct PortfolioCommand {
  WString action;          // create | info
  WString input_file;      // for info: portfolio PDF; for create: PDF file to add
  WString output_file;     // for create: output PDF path; for info: output txt path
  WString add_file;        // additional non-PDF file to add to portfolio (optional)
  WString folder_name;     // sub-folder name (default: "Sub Folder-1")
  WString folder_desc;     // sub-folder description (default: "This is a sub folder added to portfolio PDF file.")
  bool show_help;

  PortfolioCommand()
      : folder_name(L"Sub Folder-1"),
        folder_desc(L"This is a sub folder added to portfolio PDF file."),
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
  cout << "portfolio --action <create|info> --input <path> --output <path> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --action <type>                 Action: create | info." << endl;
  cout << "  --input <path>                  For 'info': portfolio PDF file path. For 'create': PDF file to add." << endl;
  cout << "  --output <path>                 For 'create': output PDF path. For 'info': output txt path." << endl << endl;
  cout << "Create options:" << endl;
  cout << "  --add-file <path>               Additional non-PDF file to add to portfolio. Optional." << endl;
  cout << "  --folder-name <text>            Sub-folder name. Default: \"Sub Folder-1\"." << endl;
  cout << "  --folder-desc <text>            Sub-folder description. Default: \"This is a sub folder added to portfolio PDF file.\"" << endl << endl;
  cout << "Optional:" << endl;
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

bool ParseCommand(int argc, char* argv[], PortfolioCommand& command) {
  if (argc <= 1) {
    PrintUsage();
    return false;
  }

  bool has_input = false;
  bool has_output = false;
  bool has_action = false;
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
    if (key.Equal("--action") || key.Equal("-a")) {
      if (!value.Equal("create") && !value.Equal("info")) {
        printf("Invalid action: %s (must be create or info)\n", (const char*)value);
        return false;
      }
      command.action = WString::FromUTF8(value);
      has_action = true;
    } else if (key.Equal("--input") || key.Equal("-i")) {
      command.input_file = WString::FromUTF8(value);
      has_input = true;
    } else if (key.Equal("--output") || key.Equal("-o")) {
      command.output_file = WString::FromUTF8(value);
      has_output = true;
    } else if (key.Equal("--add-file")) {
      command.add_file = WString::FromUTF8(value);
    } else if (key.Equal("--folder-name")) {
      command.folder_name = WString::FromUTF8(value);
    } else if (key.Equal("--folder-desc")) {
      command.folder_desc = WString::FromUTF8(value);
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (!has_action || !has_input || !has_output) {
    printf("--action, --input, and --output are all required.\n");
    PrintUsage();
    return false;
  }
  if (!FileExists(command.input_file)) {
    printf("Input file does not exist: %s\n", (const char*)String::FromUnicode(command.input_file));
    return false;
  }
  return true;
}

class TextDoc {
public:
  TextDoc(const String& file_name, const String& fill_mode);
  TextDoc(const WString& file_name, const WString& fill_mode);
  ~TextDoc();
  void Write(const char* format, ...);
  void Write(int count, const char* prefix, const char* format, ...);

private:
  FILE* file_;
};

TextDoc::TextDoc(const String& file_name, const String& file_mode) throw(Exception) : file_(NULL) {
#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file_, (const char*)file_name, (const char*)file_mode);
#else
  file_ = fopen((const char*)file_name, (const char*)file_mode);
#endif

  if (!file_)
    throw Exception(__FILE__, __LINE__, __FUNCTION__, foxit::e_ErrFile);
}

TextDoc::TextDoc(const WString& file_name, const WString& file_mode) throw(Exception)  : file_(NULL) {
  String s_file_name = String::FromUnicode(file_name);
  String s_file_mode = String::FromUnicode(file_mode);

#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file_, (const char*)s_file_name, (const char*)s_file_mode);
#else
  file_ = fopen((const char*)s_file_name, (const char*)s_file_mode);
#endif
  if (!file_)
    throw Exception(__FILE__, __LINE__, __FUNCTION__, foxit::e_ErrFile);
}

TextDoc::~TextDoc() {
  fclose(file_);
  file_ = NULL;
}

void TextDoc::Write(const char* format, ...) {
  va_list vars;
  if (file_) {
    // Get variable list
    va_start(vars, format);
    // Call vfprintf to format log data and output to log file
    vfprintf(file_, format, vars);
    // End variable list
    va_end(vars);
  }
}

void TextDoc::Write(int count, const char * prefix, const char * format, ...) {
  for (int i = 0; i < count; i++) {
    Write("%s", prefix);
  }
  va_list vars;
  if (file_) {
    // Get variable list
    va_start(vars, format);
    // Call vfprintf to format log data and output to log file
    vfprintf(file_, format, vars);
    // End variable list
    va_end(vars);
  }
}

bool CreatPortfolioPDF(const WString& input_pdf, const WString& add_file, const WString& folder_name, const WString& folder_desc, const WString& saved_path) {
  // Create a new blank portfolio PDF.
  Portfolio new_portfolio = Portfolio::CreatePortfolio();
  if (true == new_portfolio.IsEmpty()) {
    printf("[FAILED] Fail to create a new portfolio PDF.\r\n");
    return false;
  }

  // Get the root node.
  PortfolioNode root_node = new_portfolio.GetRootNode();
  if (true == root_node.IsEmpty()) {
    printf("[FAILED] Fail to get the root node.\r\n");
    return false;
  }
  // The root node should be a folder node, so transfer to use PortfolioFolderNode.
  PortfolioFolderNode root_folder(root_node);

  // Pre-load the PDF file which is to be added to new sub folder in portfolio PDF.
  // ATTENTION: please keep the PDF document object valid until portfolio PDF is saved or ends its life-cycle.
  PDFDoc pdf_doc(input_pdf);
  ErrorCode error_code = pdf_doc.Load();
  if (error_code != foxit::e_ErrSuccess) {
    printf("[FAILED] Fail to load PDF file %s. Error: %d\n", (const char*)String::FromUnicode(input_pdf), error_code);
  }

  // Add a sub folder to root folder node.
  PortfolioFolderNode new_sub_foldernode = root_folder.AddSubFolder((const wchar_t*)folder_name);
  if (true == new_sub_foldernode.IsEmpty()) {
    printf("[FAILED] Fail to add sub folder.\r\n");
  } else {
    new_sub_foldernode.SetDescription((const wchar_t*)folder_desc);

    if (false == pdf_doc.IsEmpty()) {
      // Add a valid PDF document object to current folder node.
      // ATTENTION: please keep the PDF document object valid until portfolio PDF is saved or ends its life-cycle.
      std::wstring input_pdf_filename = std::wstring((FX_LPCWSTR)input_pdf, input_pdf.GetLength());
      // Extract just the filename from the path
      size_t last_sep = input_pdf_filename.find_last_of(L'/');
      size_t last_backslash = input_pdf_filename.find_last_of(L'\\');
      int sep_pos = (last_sep > last_backslash) ? last_sep : last_backslash;
      if (sep_pos >= 0)
        input_pdf_filename = input_pdf_filename.substr(sep_pos + 1, input_pdf_filename.length() - sep_pos);

      PortfolioFileNode new_filenode = new_sub_foldernode.AddPDFDoc(pdf_doc, input_pdf_filename.c_str());
      if (true == new_filenode.IsEmpty()) {
        printf("[FAILED] Fail to add PDF file %s.\r\n", (const char*)String::FromUnicode(input_pdf));
      } else {
        FileSpec file_spec = new_filenode.GetFileSpec();
        file_spec.SetDescription("This is a common PDF file added to portfolio PDF file");
      }
    }
  }

  // Add a non-PDF file to root folder node, if specified.
  if (!add_file.IsEmpty()) {
    PortfolioFileNode new_sub_filenode = root_folder.AddFile((const wchar_t*)add_file);
    if (true == new_sub_filenode.IsEmpty()) {
      printf("[FAILED] Fail to add file %s.\r\n", (const char*)String::FromUnicode(add_file));
    } else {
      FileSpec file_spec = new_sub_filenode.GetFileSpec();
      file_spec.SetDescription("This is a non-PDF file added to portfolio PDF file.");
    }
  }
  
  // User can update schema field and other properties by class Portfolio, if necessary.
  
  // Save the new portfolio PDF file.
  PDFDoc portfolio_pdf_doc = new_portfolio.GetPortfolioPDFDoc();
  if (true == portfolio_pdf_doc.IsEmpty()) {
    printf("[FAILED] Fail to get portfolio PDF document object.\r\n");
    return false;
  } else {
    return portfolio_pdf_doc.SaveAs((const wchar_t*)saved_path, PDFDoc::e_SaveFlagNormal);
  }
}

void OutputTab(TextDoc& output_txt_doc, int nTabCount) {
  for (int i = 0; i<nTabCount; i++)
    output_txt_doc.Write("\t");
}

void OutputFileNodeInfo(TextDoc& output_txt_doc, const PortfolioFileNode& node, int tab_count) {
  if (tab_count>0)
    OutputTab(output_txt_doc, tab_count);
  output_txt_doc.Write("Type:File\r\n");

  PortfolioFileNode temp_node(node);
  WString key_name = temp_node.GetKeyName();
  if (tab_count>0)
    OutputTab(output_txt_doc, tab_count);
  output_txt_doc.Write("Key Name:%s\r\n", (const char*)String::FromUnicode(key_name));

  FileSpec file_spec = temp_node.GetFileSpec();
  WString file_name = file_spec.GetFileName();
  if (tab_count>0)
      OutputTab(output_txt_doc, tab_count);
  output_txt_doc.Write("File Name:%s\r\n", (const char*)String::FromUnicode(file_name));

  String description = file_spec.GetDescription();
  if (tab_count>0)
    OutputTab(output_txt_doc, tab_count);
  output_txt_doc.Write("Description:%s\r\n", (const char*)description);
}

void OutputSubNodesInfo(TextDoc& output_txt_doc, const PortfolioNodeArray& sub_nodes, int tab_count);

void OutputFolderNodeInfo(TextDoc& output_txt_doc, const PortfolioFolderNode& node, int tab_count) {
  if (tab_count>0)
    OutputTab(output_txt_doc, tab_count);
  output_txt_doc.Write("Type:Folder\r\n");

  PortfolioFolderNode temp_node(node);
  WString name = temp_node.GetName();
  if (tab_count>0)
    OutputTab(output_txt_doc, tab_count);
  output_txt_doc.Write("Name:%s\r\n", (const char*)String::FromUnicode(name));

  WString description = temp_node.GetDescription();
  if (tab_count>0)
    OutputTab(output_txt_doc, tab_count);
  output_txt_doc.Write("Description:%s\r\n", (const char*)String::FromUnicode(description));

  PortfolioNodeArray sub_nodes = temp_node.GetSortedSubNodes();
  OutputSubNodesInfo(output_txt_doc, sub_nodes, tab_count + 1);
}

void OutputNodeInfo(TextDoc& output_txt_doc, const PortfolioNode& node, int tab_count) {
  switch (PortfolioNode(node).GetNodeType()) {
    case PortfolioNode::e_TypeFolder:
      OutputFolderNodeInfo(output_txt_doc, PortfolioFolderNode(node), tab_count);
      break;
    case PortfolioNode::e_TypeFile:
      OutputFileNodeInfo(output_txt_doc, PortfolioFileNode(node), tab_count);
      break;
    default:
      break;
  }
}

void OutputSubNodesInfo(TextDoc& output_txt_doc, const PortfolioNodeArray& sub_nodes, int tab_count) {
  for (size_t index = 0; index < sub_nodes.GetSize(); index++) {
    if (tab_count > 0)
      OutputTab(output_txt_doc, tab_count);
    output_txt_doc.Write("Sorted Index (under current folder):%d\r\n", index);
    OutputNodeInfo(output_txt_doc, sub_nodes[index], tab_count);
    output_txt_doc.Write("========\r\n");
  }
}

void OutputSchemaFields(TextDoc& output_txt_doc, const SchemaFieldArray& field_array) {
  if (field_array.GetSize() <= 0) return;
  output_txt_doc.Write("==== Schema Fields ====\r\n");

  for (size_t i = 0; i < field_array.GetSize(); i++) {
    output_txt_doc.Write("Field index:%d\r\n", i);
    SchemaField field = field_array[i];
    if (true == field.IsEmpty()) continue;
    foxit::String key_name = field.GetKeyName();
    output_txt_doc.Write("Key name: %s\r\n", (const char*)key_name);

    foxit::String subtype_name = field.GetSubtypeName();
    output_txt_doc.Write("Subtype name: %s\r\n", (const char*)subtype_name);

    foxit::WString display_name = field.GetDisplayName();
    output_txt_doc.Write("Display name: %s\r\n", (const char*)String::FromUnicode(display_name));

    bool is_visible = field.IsVisible();
    output_txt_doc.Write("Visibility: %s\r\n", is_visible ? "true" : "false");

    output_txt_doc.Write("========\r\n");
  }
}

void OutputPortfolioProperties(TextDoc& output_txt_doc, const Portfolio& portfolio) {
  output_txt_doc.Write("==== Portfolio Properties ====\r\n");
  WString initial_filespec_keyname = portfolio.GetInitialFileSpecKeyName();
  output_txt_doc.Write("Initial FileSpec Key Name:%s\r\n", (const char*)String::FromUnicode(initial_filespec_keyname));

  Portfolio::InitialViewMode view_mode = portfolio.GetInitialViewMode();
  String view_mode_str = "";
  switch (view_mode) {
    case Portfolio::e_InitialViewUnknownMode:
      view_mode_str = "Unknown";
      break;
    case Portfolio::e_InitialViewDetailMode:
      view_mode_str = "Detail";
      break;
    case Portfolio::e_InitialViewTileMode:
      view_mode_str = "Tile";
      break;
    case Portfolio::e_InitialViewHidden:
      view_mode_str = "Hidden";
      break;
  }
  output_txt_doc.Write("Initial View Mode:%s\r\n", (const char*)view_mode_str);


  bool is_ascending = portfolio.IsSortedInAscending();
  output_txt_doc.Write("Sorting Order:%s\r\n", is_ascending? "Ascending" : "Descending" );

  String sorting_field_name = portfolio.GetSortingFieldKeyName();
  output_txt_doc.Write("Sorting Field Key Name:%s\r\n", (const char*)sorting_field_name);


  SchemaFieldArray field_array = portfolio.GetSchemaFields();
  OutputSchemaFields(output_txt_doc, field_array);
}

void OutputPortfolioPDFInfo(const WString& portfolio_file_path, TextDoc& output_txt_doc) {
  PDFDoc pdf_doc((const wchar_t*)portfolio_file_path);
  ErrorCode error_code = pdf_doc.Load();
  if (error_code != foxit::e_ErrSuccess) {
    printf("[FAILED] Fail to load Portfolio PDF file %s. Error: %d\n", (const char*)String::FromUnicode(portfolio_file_path), error_code);
    return;
  }

  if (false == pdf_doc.IsPortfolio()) {
    printf("[FAILED] Fail to output portfolio information for PDF file %s, because it is not a portfolio PDF file.\r\n", (const char*)String::FromUnicode(portfolio_file_path));
    return;
  }

  Portfolio exist_portfolio = Portfolio::CreatePortfolio(pdf_doc);
  if (true == exist_portfolio.IsEmpty()) {
    printf("[FAILED] Fail to create a portfolio object based an existed portfolio PDF document.\r\n");
    return;
  }

  // Output portfolio properties.
  OutputPortfolioProperties(output_txt_doc, exist_portfolio);
  output_txt_doc.Write("======================================================================\r\n");

  // Output all nodes.
  output_txt_doc.Write("==== Nodes Information ====\r\n");
  PortfolioNode root_node = exist_portfolio.GetRootNode();
  PortfolioFolderNode root_folder(root_node);
  PortfolioNodeArray sub_nodes = root_folder.GetSortedSubNodes();
  OutputSubNodesInfo(output_txt_doc, sub_nodes, 0);
}

int main(int argc, char *argv[])
{
  int err_ret = 0;

  PortfolioCommand command;
  if (!ParseCommand(argc, argv, command)) {
    return 1;
  }

  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    if (command.action.Equal(L"create")) {
      // Create a new portfolio PDF file
      if (CreatPortfolioPDF(command.input_file, command.add_file, command.folder_name, command.folder_desc, command.output_file)) {
        TextDoc text_doc(command.output_file + L"_info.txt", L"w+b");
        // Show information of the new portfolio PDF file.
        OutputPortfolioPDFInfo(command.output_file, text_doc);
      }
    } else if (command.action.Equal(L"info")) {
      // Output portfolio info for an existing portfolio PDF file
      TextDoc text_doc(command.output_file, L"w+b");
      OutputPortfolioPDFInfo(command.input_file, text_doc);
    }

    printf("END: Portfolio demo.\r\n");

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

