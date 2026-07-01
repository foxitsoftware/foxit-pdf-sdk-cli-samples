// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to export data, import data and reset form for
// XFA document.

// Include Foxit SDK header files.
#include <iostream>
#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/interform/fs_pdfform.h"
#include "../../../include/addon/xfa/fs_xfa.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace annots;
using namespace actions;
using namespace interform;
using namespace foxit::addon::xfa;

static const char* sn = "";
static const char* key = "";

struct XfaFormCommand {
  WString input_file;
  WString output_dir;
  WString action;           // export, import, reset
  WString data_file;        // XML data file (required for import)
  WString output_file;      // output file name (optional, auto-generated if empty)
  WString export_data_type; // xml | static_xdp | xdp (for export action, default: xml)
  bool show_help;
  XfaFormCommand() : export_data_type(L"xml"), show_help(false) {}
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
  cout << "xfa_form --input <input.pdf> --output <output_dir> --action <export|import|reset> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --input <path>                  Input PDF file path." << endl;
  cout << "  --output <path>                 Output directory path." << endl;
  cout << "  --action <action>               Action to perform: export, import, reset." << endl << endl;
  cout << "Optional:" << endl;
  cout << "  --data-file <path>              XML data file (required for import action)." << endl;
  cout << "  --output-file <name>            Output file name (without path). Auto-generated if not set." << endl;
  cout << "  --export-data-type <type>       Export data type (for export action): xml | static_xdp | xdp. Default: xml." << endl;
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

bool ParseCommand(int argc, char* argv[], XfaFormCommand& command) {
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
    if (key.Equal("--input") || key.Equal("-i")) {
      command.input_file = WString::FromUTF8(value);
      has_input = true;
    } else if (key.Equal("--output") || key.Equal("-o")) {
      command.output_dir = WString::FromUTF8(value);
      has_output = true;
    } else if (key.Equal("--action") || key.Equal("-a")) {
      command.action = WString::FromUTF8(value);
      has_action = true;
    } else if (key.Equal("--data-file") || key.Equal("-d")) {
      command.data_file = WString::FromUTF8(value);
    } else if (key.Equal("--output-file")) {
      command.output_file = WString::FromUTF8(value);
    } else if (key.Equal("--export-data-type")) {
      if (!value.Equal("xml") && !value.Equal("static_xdp") && !value.Equal("xdp")) {
        printf("Invalid export-data-type: %s (must be xml, static_xdp, or xdp)\n", (const char*)value);
        return false;
      }
      command.export_data_type = WString::FromUTF8(value);
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (!has_input || !has_output || !has_action) {
    printf("--input, --output, and --action are required.\n");
    PrintUsage();
    return false;
  }
  if (!FileExists(command.input_file)) {
    printf("Input file does not exist: %s\n", (const char*)String::FromUnicode(command.input_file));
    return false;
  }
  if (!command.action.Equal(L"export") && !command.action.Equal(L"import") && !command.action.Equal(L"reset")) {
    printf("Invalid action: %s. Must be export, import, or reset.\n", (const char*)String::FromUnicode(command.action));
    return false;
  }
  if (command.action.Equal(L"import") && command.data_file.IsEmpty()) {
    printf("--data-file is required for import action.\n");
    return false;
  }
  if (command.action.Equal(L"import") && !FileExists(command.data_file)) {
    printf("Data file does not exist: %s\n", (const char*)String::FromUnicode(command.data_file));
    return false;
  }
  return true;
}

class CFS_XFAAppHandler : public foxit::addon::xfa::AppProviderCallback
{
public:
    CFS_XFAAppHandler() {}
    ~CFS_XFAAppHandler() {}

    virtual void Release() {
        delete this;
    }

    virtual WString GetAppInfo(AppInfo app_info) {
        return L"Foxit SDK";
    }
    virtual void Beep(BeepType type) {}
    virtual AppProviderCallback::MsgBoxButtonID MsgBox(const wchar_t* message, const wchar_t* title = NULL,
        MsgBoxIconType icon_type = e_MsgBoxIconError,
        MsgBoxButtonType button_type = e_MsgBtnTypeOK) {
            return (AppProviderCallback::MsgBoxButtonID)0;
    }
    virtual WString Response(const wchar_t* question, const wchar_t* title, const wchar_t* default_answer = NULL,
        bool is_mask = true) {
            return L"answer";
    }
    virtual common::file::ReaderCallback* DownLoadUrl(const wchar_t* url) {
        return NULL;
    }
    virtual WString PostRequestURL(const wchar_t* url, const wchar_t* data, const wchar_t* content_type, const wchar_t* encode, const wchar_t* header) {
        return L"PostRequestUrl";
    }
    virtual bool PutRequestURL(const wchar_t* url, const wchar_t* data, const wchar_t* encode) {
        return TRUE;
    }
    virtual WString LoadString(AppProviderCallback::StringID string_id) {
        return L"LoadString";
    }

    virtual WStringArray ShowFileDialog(const wchar_t* string_title, const wchar_t* string_filter, bool is_openfile_dialog = TRUE) {
        return WStringArray();
    }
};

class CFS_XFADocHandler:public foxit::addon::xfa::DocProviderCallback
{
public:
    CFS_XFADocHandler() {}
    ~CFS_XFADocHandler() {}

    virtual void Release(){
        delete this;
    }
    virtual void InvalidateRect(int page_index, const RectF &rect, InvalidateFlag flag) {}
    virtual void DisplayCaret(int page_index, bool is_visible, const RectF &rect) {}
    virtual bool GetPopupPos(int page_index, float min_popup, float max_popup,
        const RectF &rect_widget, RectF &rect_popup) {
            return TRUE;
    }
    virtual bool PopupMenu(int page_index, const PointF& rect_popup) {
        return TRUE;
    }
    virtual int GetCurrentPage(const foxit::addon::xfa::XFADoc& doc) {
        return 0;
    }
    virtual void SetCurrentPage(const foxit::addon::xfa::XFADoc& doc, int current_page_index) {}
    virtual WString GetTitle(const foxit::addon::xfa::XFADoc& doc) {
        return L"title";
    }
    virtual void ExportData(const foxit::addon::xfa::XFADoc& doc, const WString& file_path) {}
    virtual void ImportData(const foxit::addon::xfa::XFADoc& doc, const WString& file_path) {}
    virtual void GotoURL(const foxit::addon::xfa::XFADoc& doc, const WString& url) {}
    virtual void Print(const foxit::addon::xfa::XFADoc& doc, int start_page_index, int end_page_index, uint32 options) {}
    virtual ARGB GetHighlightColor(const foxit::addon::xfa::XFADoc& doc) {
        if(doc.GetType() == foxit::addon::xfa::XFADoc::e_Static) {
            return 0x50FF0000;
        }
        else {
            return 0x500000FF;
        }
    }
    virtual bool SubmitData(const foxit::addon::xfa::XFADoc& doc, const WString& target, SubmitFormat format, TextEncoding text_encoding, const WString& content) {
        return TRUE;
    }
    virtual void SetFocus(foxit::addon::xfa::XFAWidget& xfa_widget, bool is_relayout) {};
    virtual void PageViewEvent(int page_index, PageViewEventType page_view_event_type) {}
    virtual void SetChangeMark(const foxit::addon::xfa::XFADoc& doc) {}
    virtual void WidgetEvent(const foxit::addon::xfa::XFAWidget& xfa_widget,WidgetEventType widget_event_type) {}
    virtual void NotifyWidgetChangeInfo(const foxit::addon::xfa::XFADoc& doc, foxit::addon::xfa::XFAWidgetModifyInfo change_info) {}
};

int main(int argc, char *argv[])
{
  XfaFormCommand command;
  if (!ParseCommand(argc, argv, command)) {
    return 1;
  }

  WString output_directory = command.output_dir;
  if (!output_directory.IsEmpty() && output_directory.GetAt((int)output_directory.GetLength() - 1) != L'/' &&
      output_directory.GetAt((int)output_directory.GetLength() - 1) != L'\\') {
    output_directory += L"/";
  }

#if defined(_WIN32) || defined(_WIN64)
  _mkdir(String::FromUnicode(output_directory));
#else
  mkdir(String::FromUnicode(output_directory), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    CFS_XFAAppHandler* pXFAAppHandler = new CFS_XFAAppHandler();
    Library::RegisterXFAAppProviderCallback(pXFAAppHandler);
    PDFDoc doc(command.input_file);
    ErrorCode error_code = doc.Load();
    if (error_code != foxit::e_ErrSuccess) {
        printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(command.input_file), error_code);
        return 1;
    }

    CFS_XFADocHandler* pXFADocHandler = new CFS_XFADocHandler();
    XFADoc xfa_doc(doc, pXFADocHandler);
    xfa_doc.StartLoad(NULL);

    if (command.action.Equal(L"export")) {
      XFADoc::ExportDataType export_type = XFADoc::e_ExportDataTypeXML;
      WString ext = L".xml";
      if (command.export_data_type.Equal(L"static_xdp")) {
        export_type = XFADoc::e_ExportDataTypeStaticXDP;
        ext = L".xdp";
      } else if (command.export_data_type.Equal(L"xdp")) {
        export_type = XFADoc::e_ExportDataTypeXDP;
        ext = L".xdp";
      }
      WString out_name = command.output_file.IsEmpty() ? (L"xfa_form" + ext) : command.output_file;
      WString out_path = output_directory + out_name;
      xfa_doc.ExportData(out_path, export_type);
      printf("XFA data exported to: %s\n", (const char*)String::FromUnicode(out_path));
    } else if (command.action.Equal(L"reset")) {
      WString out_name = command.output_file.IsEmpty() ? L"xfa_dynamic_resetform.pdf" : command.output_file;
      WString out_path = output_directory + out_name;
      xfa_doc.ResetForm();
      doc.SaveAs(out_path);
      printf("XFA form reset, saved to: %s\n", (const char*)String::FromUnicode(out_path));
    } else if (command.action.Equal(L"import")) {
      WString out_name = command.output_file.IsEmpty() ? L"xfa_dynamic_importdata.pdf" : command.output_file;
      WString out_path = output_directory + out_name;
      xfa_doc.ImportData(command.data_file);
      doc.SaveAs(out_path);
      printf("XFA data imported from: %s, saved to: %s\n",
             (const char*)String::FromUnicode(command.data_file),
             (const char*)String::FromUnicode(out_path));
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
