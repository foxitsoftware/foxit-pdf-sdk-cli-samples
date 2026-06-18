// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to extract attached files from PDF files and
// add files as attachments to PDF files.

// Include Foxit SDK header files.
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/fs_filespec.h"
#include "../../../include/pdf/fs_pdfattachments.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "";
static const char* key = "";

static std::string g_action;
static std::string g_input_file;
static std::string g_output_path;
static std::string g_key;
static std::string g_attach_file;
static std::string g_new_key;

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

static std::string GetBasename(const std::string& path) {
  size_t sep = path.find_last_of("/\\");
  return (sep != std::string::npos) ? path.substr(sep + 1) : path;
}

static bool HasAttachmentKey(Attachments& attachments, const WString& key_name) {
  int count = attachments.GetCount();
  for (int i = 0; i < count; ++i) {
    if (attachments.GetKey(i).Equal(key_name))
      return true;
  }
  return false;
}

static bool ParseArgs(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    foxit::String arg(argv[i]);
    if (arg.Equal("--help")) {
      printf(
        "Usage: attachment_xxx [options]\n"
        "\n"
        "Required options:\n"
        "  --action <op>        Operation: get, add, delete, edit\n"
        "  -i <file>            Input PDF file path\n"
        "  -o <file>            Output path\n"
        "                        get: exported attachment path\n"
        "                        add/delete/edit: output PDF path\n"
        "\n"
        "Action-specific options:\n"
        "  --key <name>         Attachment key (required for get by key, delete, edit, and add)\n"
        "  --attach <file>      Local file path (required for add and edit)\n"
        "  --new-key <name>     Optional new key when editing\n"
        "\n"
        "Examples:\n"
        "  --action get -i in.pdf -o out.bin --key file1\n"
        "  --action add -i in.pdf -o out.pdf --key new_file --attach data.bin\n"
        "  --action delete -i in.pdf -o out.pdf --key old_file\n"
        "  --action edit -i in.pdf -o out.pdf --key file1 --attach new.bin --new-key file2\n"
      );
      exit(0);
    }

    if (arg.Equal("--action")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--action'.\nTry 'attachment_xxx --help' for more information.\n");
        return false;
      }
      ++i;
      foxit::String v(argv[i]);
      if (!v.Equal("get") && !v.Equal("add") && !v.Equal("delete") && !v.Equal("edit")) {
        printf("Invalid --action '%s'. Expected: get, add, delete, or edit\nTry 'attachment_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_action = argv[i];
    } else if (arg.Equal("-i")) {
      if (i + 1 >= argc) {
        printf("Missing value for '-i'.\nTry 'attachment_xxx --help' for more information.\n");
        return false;
      }
      ++i;
      g_input_file = argv[i];
    } else if (arg.Equal("-o")) {
      if (i + 1 >= argc) {
        printf("Missing value for '-o'.\nTry 'attachment_xxx --help' for more information.\n");
        return false;
      }
      ++i;
      g_output_path = argv[i];
    } else if (arg.Equal("--key")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--key'.\nTry 'attachment_xxx --help' for more information.\n");
        return false;
      }
      ++i;
      g_key = argv[i];
    } else if (arg.Equal("--attach")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--attach'.\nTry 'attachment_xxx --help' for more information.\n");
        return false;
      }
      ++i;
      g_attach_file = argv[i];
    } else if (arg.Equal("--new-key")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--new-key'.\nTry 'attachment_xxx --help' for more information.\n");
        return false;
      }
      ++i;
      g_new_key = argv[i];
    } else {
      printf("Unknown argument: '%s'.\nTry 'attachment_xxx --help' for more information.\n", argv[i]);
      return false;
    }
  }
  return true;
}

static int GetAttachment(Attachments& attachments) {
  if (g_key.empty()) {
    int count = attachments.GetCount();
    printf("Attachment count: %d\n", count);
    for (int i = 0; i < count; ++i) {
      WString key_name = attachments.GetKey(i);
      FileSpec spec = attachments.GetEmbeddedFile(key_name);
      printf("[%d] key=%s, file=%s, size=%d\n",
        i,
        (const char*)String::FromUnicode(key_name),
        (const char*)String::FromUnicode(spec.GetFileName()),
        spec.GetFileSize());
    }
    return 0;
  }

  WString key_name = WString::FromLocal(g_key.c_str());
  if (!HasAttachmentKey(attachments, key_name)) {
    printf("Error: key '%s' not found.\n", g_key.c_str());
    return 1;
  }
  bool ok = attachments.ExtractEmbeddedFileTo(key_name, WString::FromLocal(g_output_path.c_str()));
  if (!ok) {
    printf("Error: failed to extract key '%s' to '%s'.\n", g_key.c_str(), g_output_path.c_str());
    return 1;
  }
  printf("Extracted key '%s' to '%s'.\n", g_key.c_str(), g_output_path.c_str());
  return 0;
}

static int AddAttachment(PDFDoc& doc, Attachments& attachments) {
  if (g_key.empty()) {
    printf("Error: --key is required for add.\n");
    return 1;
  }
  if (g_attach_file.empty()) {
    printf("Error: --attach is required for add.\n");
    return 1;
  }

  WString key_name = WString::FromLocal(g_key.c_str());
  if (HasAttachmentKey(attachments, key_name)) {
    printf("Error: key '%s' already exists.\n", g_key.c_str());
    return 1;
  }

  bool ok = attachments.AddFromFilePath(key_name, WString::FromLocal(g_attach_file.c_str()));
  if (!ok) {
    printf("Error: failed to add attachment from '%s'.\n", g_attach_file.c_str());
    return 1;
  }

  WString output_pdf = WString::FromLocal(g_output_path.c_str());
  doc.SaveAs(output_pdf, PDFDoc::e_SaveFlagNoOriginal);
  printf("Added key '%s' from '%s', saved PDF to '%s'.\n", g_key.c_str(), g_attach_file.c_str(), g_output_path.c_str());
  return 0;
}

static int DeleteAttachment(PDFDoc& doc, Attachments& attachments) {
  if (g_key.empty()) {
    printf("Error: --key is required for delete.\n");
    return 1;
  }

  WString key_name = WString::FromLocal(g_key.c_str());
  if (!HasAttachmentKey(attachments, key_name)) {
    printf("Error: key '%s' not found.\n", g_key.c_str());
    return 1;
  }

  bool ok = attachments.RemoveEmbeddedFile(key_name);
  if (!ok) {
    printf("Error: failed to delete key '%s'.\n", g_key.c_str());
    return 1;
  }

  WString output_pdf = WString::FromLocal(g_output_path.c_str());
  doc.SaveAs(output_pdf, PDFDoc::e_SaveFlagNoOriginal);
  printf("Deleted key '%s', saved PDF to '%s'.\n", g_key.c_str(), g_output_path.c_str());
  return 0;
}

static int EditAttachment(PDFDoc& doc, Attachments& attachments) {
  if (g_key.empty()) {
    printf("Error: --key is required for edit.\n");
    return 1;
  }
  if (g_attach_file.empty()) {
    printf("Error: --attach is required for edit.\n");
    return 1;
  }

  WString key_name = WString::FromLocal(g_key.c_str());
  if (!HasAttachmentKey(attachments, key_name)) {
    printf("Error: key '%s' not found.\n", g_key.c_str());
    return 1;
  }

  FileSpec new_spec(doc);
  std::string basename = GetBasename(g_attach_file);
  new_spec.SetFileName(WString::FromLocal(basename.c_str()));
  new_spec.Embed(WString::FromLocal(g_attach_file.c_str()));
  new_spec.SetSubtype();

  if (g_new_key.empty() || g_new_key == g_key) {
    bool ok = attachments.SetEmbeddedFile(key_name, new_spec);
    if (!ok) {
      printf("Error: failed to replace content for key '%s'.\n", g_key.c_str());
      return 1;
    }
  } else {
    WString dst_key = WString::FromLocal(g_new_key.c_str());
    if (HasAttachmentKey(attachments, dst_key)) {
      printf("Error: --new-key '%s' already exists.\n", g_new_key.c_str());
      return 1;
    }
    bool ok_add = attachments.AddEmbeddedFile(dst_key, new_spec);
    if (!ok_add) {
      printf("Error: failed to add edited attachment to new key '%s'.\n", g_new_key.c_str());
      return 1;
    }
    bool ok_del = attachments.RemoveEmbeddedFile(key_name);
    if (!ok_del) {
      printf("Error: failed to remove old key '%s' after add.\n", g_key.c_str());
      return 1;
    }
  }

  WString output_pdf = WString::FromLocal(g_output_path.c_str());
  doc.SaveAs(output_pdf, PDFDoc::e_SaveFlagNoOriginal);
  printf("Edited key '%s' using '%s', saved PDF to '%s'.\n", g_key.c_str(), g_attach_file.c_str(), g_output_path.c_str());
  return 0;
}

int main(int argc, char *argv[]) {
  if (!ParseArgs(argc, argv)) return 1;

  if (g_action.empty()) {
    printf("Error: --action is required.\nTry 'attachment_xxx --help' for more information.\n");
    return 1;
  }
  if (g_input_file.empty()) {
    printf("Error: -i (input file) is required.\nTry 'attachment_xxx --help' for more information.\n");
    return 1;
  }
  if (g_output_path.empty()) {
    printf("Error: -o (output path) is required.\nTry 'attachment_xxx --help' for more information.\n");
    return 1;
  }

  if ((g_action == "add" || g_action == "delete" || g_action == "edit")) {
    if (g_output_path.size() < 4 ||
        g_output_path.substr(g_output_path.size() - 4) != ".pdf") {
      printf("Warning: for %s action, -o should typically be a .pdf file.\n", g_action.c_str());
    }
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  {
    std::size_t osep = g_output_path.find_last_of("/\\");
    if (osep != std::string::npos) {
      std::string out_parent = g_output_path.substr(0, osep);
#if defined(_WIN32) || defined(_WIN64)
      _mkdir(out_parent.c_str());
#else
      mkdir(out_parent.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    }
  }

  try {
    WString input_file = WString::FromLocal(g_input_file.c_str());
    PDFDoc doc = PDFDoc(input_file);
    ErrorCode error_code = doc.Load();
    if (error_code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", g_input_file.c_str(), error_code);
      return 1;
    }
    Attachments attachments(doc);
    if (g_action == "get") {
      err_ret = GetAttachment(attachments);
    } else if (g_action == "add") {
      err_ret = AddAttachment(doc, attachments);
    } else if (g_action == "delete") {
      err_ret = DeleteAttachment(doc, attachments);
    } else if (g_action == "edit") {
      err_ret = EditAttachment(doc, attachments);
    }

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
