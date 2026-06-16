
// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to associate files with PDF.

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
#include "../../../include/pdf/fs_filespec.h"
#include "../../../include/pdf/fs_pdfassociatefiles.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace pdf::graphics;
using namespace pdf::objects;
using namespace annots;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

// CLI parameter globals
static std::string g_action;         // get, add, delete, edit
static std::string g_command;        // catalog, page, annot, image, form, marked-content
static std::string g_input_file;
static std::string g_output_path;
static int g_page_index = 0;
static int g_index = 0;              // AF index (catalog/page/image/form/marked-content) or annot index (annot command)
static std::string g_attach_file;   // local file to embed (add/edit)

class SdkLibMgr {
public:
  SdkLibMgr() : is_initialize_(false) {}
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

static std::string GetBasename(const std::string& path) {
  size_t sep = path.find_last_of("/\\");
  return (sep != std::string::npos) ? path.substr(sep + 1) : path;
}

static bool ParseArgs(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    foxit::String arg(argv[i]);
    if (arg.Equal("--help")) {
      printf(
        "Usage: associated_files_xxx [options]\n"
        "\n"
        "Required options:\n"
        "  -i <file>                  Input PDF file path\n"
        "  -o <file>                  Output file path\n"
        "                               For --action get: path where the extracted AF is saved\n"
        "                               For --action add/delete/edit: path of the saved output PDF\n"
        "  --action <action>          Operation: get, add, delete, or edit\n"
        "  --command <type>           Target PDF object type:\n"
        "                               catalog        - document catalog dictionary\n"
        "                               page           - page dictionary\n"
        "                               annot          - annotation\n"
        "                               image          - first image XObject on the page\n"
        "                               form           - first form XObject on the page\n"
        "                               marked-content - first text object's marked content\n"
        "\n"
        "Optional options:\n"
        "  --page <N>                 0-based page index (default: 0)\n"
        "                               Used by: page, annot, image, form, marked-content\n"
        "  --index <N>                0-based index (default: 0)\n"
        "                               For annot: which annotation to target\n"
        "                               For marked-content: which AF item to get/delete/edit\n"
        "                               For catalog/page/image/form: which AF to get/delete/edit\n"
        "  --attach <file>            Local file to embed (required for add and edit)\n"
        "  --help                     Show this help and exit\n"
        "\n"
        "Examples:\n"
        "  --action get --command catalog -i in.pdf -o out.txt\n"
        "  --action get --command annot -i in.pdf -o out.txt --page 0 --index 0\n"
        "  --action add --command page -i in.pdf -o out.pdf --page 0 --attach 1.txt\n"
        "  --action delete --command catalog -i in.pdf -o out.pdf --index 0\n"
        "  --action edit --command catalog -i in.pdf -o out.pdf --index 0 --attach new.txt\n"
      );
      exit(0);
    }

#define NEED_VAL(opt) \
    do { if (i + 1 >= argc) { \
      printf("Missing value for '%s'.\nTry 'associated_files_xxx --help' for more information.\n", (opt)); \
      return false; \
    } } while(0)

    if (arg.Equal("-i")) {
      NEED_VAL("-i"); ++i;
      g_input_file = argv[i];
    } else if (arg.Equal("-o")) {
      NEED_VAL("-o"); ++i;
      g_output_path = argv[i];
    } else if (arg.Equal("--action")) {
      NEED_VAL("--action"); ++i;
      foxit::String v(argv[i]);
      if (!v.Equal("get") && !v.Equal("add") && !v.Equal("delete") && !v.Equal("edit")) {
        printf("Invalid --action '%s'. Expected: get, add, delete, or edit\nTry 'associated_files_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_action = argv[i];
    } else if (arg.Equal("--command")) {
      NEED_VAL("--command"); ++i;
      foxit::String v(argv[i]);
      if (!v.Equal("catalog") && !v.Equal("page") && !v.Equal("annot") &&
          !v.Equal("image") && !v.Equal("form") && !v.Equal("marked-content")) {
        printf("Invalid --command '%s'. Expected: catalog, page, annot, image, form, or marked-content\nTry 'associated_files_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_command = argv[i];
    } else if (arg.Equal("--page")) {
      NEED_VAL("--page"); ++i;
      g_page_index = atoi(argv[i]);
    } else if (arg.Equal("--index")) {
      NEED_VAL("--index"); ++i;
      g_index = atoi(argv[i]);
    } else if (arg.Equal("--attach")) {
      NEED_VAL("--attach"); ++i;
      g_attach_file = argv[i];
    } else {
      printf("Unknown argument: '%s'.\nTry 'associated_files_xxx --help' for more information.\n", argv[i]);
      return false;
    }
  }
  return true;
}

// Get (extract) AF at g_index from the target object and export to g_output_path.
static int GetAssociatedFile(PDFDoc& doc) {
  AssociatedFiles af_mgr(doc);
  WString out = WString::FromLocal(g_output_path.c_str());

  if (g_command == "catalog") {
    PDFDictionary* dict = doc.GetCatalog();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (g_index >= count) {
      printf("Error: --index %d out of range, catalog has %d AF(s).\n", g_index, count);
      return 1;
    }
    FileSpec filespec = af_mgr.GetAssociatedFile(dict, g_index);
    filespec.ExportToFile(out);
    printf("Exported catalog AF[%d] to \"%s\".\n", g_index, g_output_path.c_str());
  } else if (g_command == "page") {
    PDFPage page = doc.GetPage(g_page_index);
    PDFDictionary* dict = page.GetDict();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (g_index >= count) {
      printf("Error: --index %d out of range, page %d has %d AF(s).\n", g_index, g_page_index, count);
      return 1;
    }
    FileSpec filespec = af_mgr.GetAssociatedFile(dict, g_index);
    filespec.ExportToFile(out);
    printf("Exported page %d AF[%d] to \"%s\".\n", g_page_index, g_index, g_output_path.c_str());
  } else if (g_command == "annot") {
    PDFPage page = doc.GetPage(g_page_index);
    Annot annot = page.GetAnnot(g_index);
    PDFDictionary* dict = annot.GetDict();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (count == 0) {
      printf("Error: annot %d on page %d has no associated files.\n", g_index, g_page_index);
      return 1;
    }
    FileSpec filespec = af_mgr.GetAssociatedFile(dict, 0);
    filespec.ExportToFile(out);
    printf("Exported annot %d on page %d AF[0] to \"%s\".\n", g_index, g_page_index, g_output_path.c_str());
  } else if (g_command == "image") {
    PDFPage page = doc.GetPage(g_page_index);
    page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
    POSITION pos = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeImage);
    GraphicsObject* obj = page.GetGraphicsObject(pos);
    if (!obj) {
      printf("Error: no image XObject found on page %d.\n", g_page_index);
      return 1;
    }
    PDFDictionary* dict = ((ImageObject*)obj)->GetStream()->GetDictionary();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (g_index >= count) {
      printf("Error: --index %d out of range, image XObject has %d AF(s).\n", g_index, count);
      return 1;
    }
    FileSpec filespec = af_mgr.GetAssociatedFile(dict, g_index);
    filespec.ExportToFile(out);
    printf("Exported image XObject AF[%d] on page %d to \"%s\".\n", g_index, g_page_index, g_output_path.c_str());
  } else if (g_command == "form") {
    PDFPage page = doc.GetPage(g_page_index);
    page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
    POSITION pos = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeFormXObject);
    GraphicsObject* obj = page.GetGraphicsObject(pos);
    if (!obj) {
      printf("Error: no form XObject found on page %d.\n", g_page_index);
      return 1;
    }
    PDFDictionary* dict = ((FormXObject*)obj)->GetStream()->GetDictionary();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (g_index >= count) {
      printf("Error: --index %d out of range, form XObject has %d AF(s).\n", g_index, count);
      return 1;
    }
    FileSpec filespec = af_mgr.GetAssociatedFile(dict, g_index);
    filespec.ExportToFile(out);
    printf("Exported form XObject AF[%d] on page %d to \"%s\".\n", g_index, g_page_index, g_output_path.c_str());
  } else if (g_command == "marked-content") {
    PDFPage page = doc.GetPage(g_page_index);
    page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
    POSITION pos = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeText);
    GraphicsObject* obj = page.GetGraphicsObject(pos);
    if (!obj) {
      printf("Error: no text object found on page %d.\n", g_page_index);
      return 1;
    }
    int count = af_mgr.GetAssociatedFilesCount(obj);
    if (g_index >= count) {
      printf("Error: --index %d out of range, text object has %d AF(s).\n", g_index, count);
      return 1;
    }
    FileSpec filespec = af_mgr.GetAssociatedFile(obj, g_index);
    filespec.ExportToFile(out);
    printf("Exported text object marked-content AF[%d] on page %d to \"%s\".\n", g_index, g_page_index, g_output_path.c_str());
  }
  return 0;
}

// Add: embed g_attach_file and associate with the target object.
static int AddAssociatedFile(PDFDoc& doc) {
  AssociatedFiles af_mgr(doc);
  WString attach = WString::FromLocal(g_attach_file.c_str());
  std::string basename = GetBasename(g_attach_file);

  FileSpec filespec(doc);
  filespec.SetAssociteFileRelationship(AssociatedFiles::e_RelationshipSource);
  filespec.SetFileName(WString::FromLocal(basename.c_str()));
  filespec.Embed(attach);
  filespec.SetSubtype();

  if (g_command == "catalog") {
    PDFObject* catalog = doc.GetCatalog();
    af_mgr.AssociateFile(catalog, filespec);
    printf("Associated \"%s\" with catalog dictionary.\n", basename.c_str());
  } else if (g_command == "page") {
    PDFPage page = doc.GetPage(g_page_index);
    page.StartParse();
    af_mgr.AssociateFile(page, filespec);
    printf("Associated \"%s\" with page %d.\n", basename.c_str(), g_page_index);
  } else if (g_command == "annot") {
    PDFPage page = doc.GetPage(g_page_index);
    page.StartParse();
    Annot annot = page.GetAnnot(g_index);
    af_mgr.AssociateFile(annot, filespec);
    printf("Associated \"%s\" with annot %d on page %d.\n", basename.c_str(), g_index, g_page_index);
  } else if (g_command == "image") {
    PDFPage page = doc.GetPage(g_page_index);
    page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
    POSITION pos = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeImage);
    ImageObject* obj = (ImageObject*)page.GetGraphicsObject(pos);
    if (!obj) {
      printf("Error: no image XObject found on page %d.\n", g_page_index);
      return 1;
    }
    af_mgr.AssociateFile(obj, filespec);
    printf("Associated \"%s\" with image XObject on page %d.\n", basename.c_str(), g_page_index);
  } else if (g_command == "form") {
    PDFPage page = doc.GetPage(g_page_index);
    page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
    POSITION pos = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeFormXObject);
    FormXObject* obj = (FormXObject*)page.GetGraphicsObject(pos);
    if (!obj) {
      printf("Error: no form XObject found on page %d.\n", g_page_index);
      return 1;
    }
    af_mgr.AssociateFile(obj, filespec);
    printf("Associated \"%s\" with form XObject on page %d.\n", basename.c_str(), g_page_index);
  } else if (g_command == "marked-content") {
    PDFPage page = doc.GetPage(g_page_index);
    page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
    PDFDictionary* page_dict = page.GetDict();
    PDFDictionary* resource_dict = (PDFDictionary*)page_dict->GetElement("Resources");
    if (!resource_dict) {
      printf("Error: no Resources dictionary found on page %d.\n", g_page_index);
      return 1;
    }
    POSITION pos = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeText);
    GraphicsObject* text_obj = page.GetGraphicsObject(pos);
    if (!text_obj) {
      printf("Error: no text object found on page %d.\n", g_page_index);
      return 1;
    }
    MarkedContent* mc = text_obj->GetMarkedContent();
    if (!mc) {
      printf("Error: no marked content found on text object.\n");
      return 1;
    }
    if (mc->GetItemCount() == 0) {
      mc->AddItem("Associated");
    }
    af_mgr.AssociateFile(text_obj, 0, resource_dict, "associated_file", filespec);
    page.GenerateContent();
    printf("Associated \"%s\" with text object marked-content on page %d.\n", basename.c_str(), g_page_index);
  }
  return 0;
}

// Delete: remove AF at g_index from the target object.
static int DeleteAssociatedFile(PDFDoc& doc) {
  AssociatedFiles af_mgr(doc);

  if (g_command == "catalog") {
    PDFDictionary* dict = doc.GetCatalog();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (g_index >= count) {
      printf("Error: --index %d out of range, catalog has %d AF(s).\n", g_index, count);
      return 1;
    }
    af_mgr.RemoveAssociatedFile(dict, g_index);
    printf("Removed catalog AF[%d].\n", g_index);
  } else if (g_command == "page") {
    PDFPage page = doc.GetPage(g_page_index);
    PDFDictionary* dict = page.GetDict();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (g_index >= count) {
      printf("Error: --index %d out of range, page %d has %d AF(s).\n", g_index, g_page_index, count);
      return 1;
    }
    af_mgr.RemoveAssociatedFile(dict, g_index);
    printf("Removed page %d AF[%d].\n", g_page_index, g_index);
  } else if (g_command == "annot") {
    PDFPage page = doc.GetPage(g_page_index);
    Annot annot = page.GetAnnot(g_index);
    PDFDictionary* dict = annot.GetDict();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (count == 0) {
      printf("Error: annot %d on page %d has no associated files.\n", g_index, g_page_index);
      return 1;
    }
    af_mgr.RemoveAssociatedFile(dict, 0);
    printf("Removed annot %d on page %d AF[0].\n", g_index, g_page_index);
  } else if (g_command == "image") {
    PDFPage page = doc.GetPage(g_page_index);
    page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
    POSITION pos = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeImage);
    GraphicsObject* obj = page.GetGraphicsObject(pos);
    if (!obj) {
      printf("Error: no image XObject found on page %d.\n", g_page_index);
      return 1;
    }
    PDFDictionary* dict = ((ImageObject*)obj)->GetStream()->GetDictionary();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (g_index >= count) {
      printf("Error: --index %d out of range, image XObject has %d AF(s).\n", g_index, count);
      return 1;
    }
    af_mgr.RemoveAssociatedFile(dict, g_index);
    printf("Removed image XObject AF[%d] on page %d.\n", g_index, g_page_index);
  } else if (g_command == "form") {
    PDFPage page = doc.GetPage(g_page_index);
    page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
    POSITION pos = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeFormXObject);
    GraphicsObject* obj = page.GetGraphicsObject(pos);
    if (!obj) {
      printf("Error: no form XObject found on page %d.\n", g_page_index);
      return 1;
    }
    PDFDictionary* dict = ((FormXObject*)obj)->GetStream()->GetDictionary();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (g_index >= count) {
      printf("Error: --index %d out of range, form XObject has %d AF(s).\n", g_index, count);
      return 1;
    }
    af_mgr.RemoveAssociatedFile(dict, g_index);
    printf("Removed form XObject AF[%d] on page %d.\n", g_index, g_page_index);
  } else if (g_command == "marked-content") {
    PDFPage page = doc.GetPage(g_page_index);
    page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
    POSITION pos = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeText);
    GraphicsObject* obj = page.GetGraphicsObject(pos);
    if (!obj) {
      printf("Error: no text object found on page %d.\n", g_page_index);
      return 1;
    }
    int count = af_mgr.GetAssociatedFilesCount(obj);
    if (g_index >= count) {
      printf("Error: --index %d out of range, text object has %d AF(s).\n", g_index, count);
      return 1;
    }
    af_mgr.RemoveAssociatedFile(obj, g_index);
    printf("Removed text object marked-content AF[%d] on page %d.\n", g_index, g_page_index);
  }
  return 0;
}

// Edit: re-embed g_attach_file into existing AF at g_index on the target object.
static int EditAssociatedFile(PDFDoc& doc) {
  AssociatedFiles af_mgr(doc);
  WString attach = WString::FromLocal(g_attach_file.c_str());

  if (g_command == "catalog") {
    PDFDictionary* dict = doc.GetCatalog();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (g_index >= count) {
      printf("Error: --index %d out of range, catalog has %d AF(s).\n", g_index, count);
      return 1;
    }
    FileSpec filespec = af_mgr.GetAssociatedFile(dict, g_index);
    filespec.Embed(attach);
    printf("Re-embedded catalog AF[%d] with \"%s\".\n", g_index, g_attach_file.c_str());
  } else if (g_command == "page") {
    PDFPage page = doc.GetPage(g_page_index);
    PDFDictionary* dict = page.GetDict();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (g_index >= count) {
      printf("Error: --index %d out of range, page %d has %d AF(s).\n", g_index, g_page_index, count);
      return 1;
    }
    FileSpec filespec = af_mgr.GetAssociatedFile(dict, g_index);
    filespec.Embed(attach);
    printf("Re-embedded page %d AF[%d] with \"%s\".\n", g_page_index, g_index, g_attach_file.c_str());
  } else if (g_command == "annot") {
    PDFPage page = doc.GetPage(g_page_index);
    Annot annot = page.GetAnnot(g_index);
    PDFDictionary* dict = annot.GetDict();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (count == 0) {
      printf("Error: annot %d on page %d has no associated files.\n", g_index, g_page_index);
      return 1;
    }
    FileSpec filespec = af_mgr.GetAssociatedFile(dict, 0);
    filespec.Embed(attach);
    printf("Re-embedded annot %d on page %d AF[0] with \"%s\".\n", g_index, g_page_index, g_attach_file.c_str());
  } else if (g_command == "image") {
    PDFPage page = doc.GetPage(g_page_index);
    page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
    POSITION pos = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeImage);
    GraphicsObject* obj = page.GetGraphicsObject(pos);
    if (!obj) {
      printf("Error: no image XObject found on page %d.\n", g_page_index);
      return 1;
    }
    PDFDictionary* dict = ((ImageObject*)obj)->GetStream()->GetDictionary();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (g_index >= count) {
      printf("Error: --index %d out of range, image XObject has %d AF(s).\n", g_index, count);
      return 1;
    }
    FileSpec filespec = af_mgr.GetAssociatedFile(dict, g_index);
    filespec.Embed(attach);
    printf("Re-embedded image XObject AF[%d] on page %d with \"%s\".\n", g_index, g_page_index, g_attach_file.c_str());
  } else if (g_command == "form") {
    PDFPage page = doc.GetPage(g_page_index);
    page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
    POSITION pos = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeFormXObject);
    GraphicsObject* obj = page.GetGraphicsObject(pos);
    if (!obj) {
      printf("Error: no form XObject found on page %d.\n", g_page_index);
      return 1;
    }
    PDFDictionary* dict = ((FormXObject*)obj)->GetStream()->GetDictionary();
    int count = af_mgr.GetAssociatedFilesCount(dict);
    if (g_index >= count) {
      printf("Error: --index %d out of range, form XObject has %d AF(s).\n", g_index, count);
      return 1;
    }
    FileSpec filespec = af_mgr.GetAssociatedFile(dict, g_index);
    filespec.Embed(attach);
    printf("Re-embedded form XObject AF[%d] on page %d with \"%s\".\n", g_index, g_page_index, g_attach_file.c_str());
  } else if (g_command == "marked-content") {
    PDFPage page = doc.GetPage(g_page_index);
    page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
    POSITION pos = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeText);
    GraphicsObject* obj = page.GetGraphicsObject(pos);
    if (!obj) {
      printf("Error: no text object found on page %d.\n", g_page_index);
      return 1;
    }
    int count = af_mgr.GetAssociatedFilesCount(obj);
    if (g_index >= count) {
      printf("Error: --index %d out of range, text object has %d AF(s).\n", g_index, count);
      return 1;
    }
    FileSpec filespec = af_mgr.GetAssociatedFile(obj, g_index);
    filespec.Embed(attach);
    printf("Re-embedded text object marked-content AF[%d] on page %d with \"%s\".\n", g_index, g_page_index, g_attach_file.c_str());
  }
  return 0;
}

int main(int argc, char* argv[]) {
  if (!ParseArgs(argc, argv)) return 1;

  // Validate required arguments
  if (g_action.empty()) {
    printf("Error: --action is required.\nTry 'associated_files_xxx --help' for more information.\n");
    return 1;
  }
  if (g_command.empty()) {
    printf("Error: --command is required.\nTry 'associated_files_xxx --help' for more information.\n");
    return 1;
  }
  if (g_input_file.empty()) {
    printf("Error: -i (input file) is required.\nTry 'associated_files_xxx --help' for more information.\n");
    return 1;
  }
  if (g_output_path.empty()) {
    printf("Error: -o (output path) is required.\nTry 'associated_files_xxx --help' for more information.\n");
    return 1;
  }
  if ((g_action == "add" || g_action == "edit") && g_attach_file.empty()) {
    printf("Error: --attach is required for --action %s.\nTry 'associated_files_xxx --help' for more information.\n", g_action.c_str());
    return 1;
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    WString input_file = WString::FromLocal(g_input_file.c_str());
    WString output_file = WString::FromLocal(g_output_path.c_str());

    // Create output parent directory if needed
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

    PDFDoc doc(input_file);
    error_code = doc.Load();
    if (error_code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", g_input_file.c_str(), error_code);
      return 1;
    }

    if (g_action == "get") {
      err_ret = GetAssociatedFile(doc);
    } else if (g_action == "add") {
      err_ret = AddAssociatedFile(doc);
      if (err_ret == 0)
        doc.SaveAs(output_file, PDFDoc::e_SaveFlagNoOriginal);
    } else if (g_action == "delete") {
      err_ret = DeleteAssociatedFile(doc);
      if (err_ret == 0)
        doc.SaveAs(output_file, PDFDoc::e_SaveFlagNoOriginal);
    } else if (g_action == "edit") {
      err_ret = EditAssociatedFile(doc);
      if (err_ret == 0)
        doc.SaveAs(output_file, PDFDoc::e_SaveFlagNoOriginal);
    }
  }
  catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  }
  catch (...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}
