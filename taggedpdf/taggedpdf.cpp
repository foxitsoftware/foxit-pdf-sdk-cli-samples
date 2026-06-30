// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to tag a PDF document.

// Include Foxit SDK header files.
#include <iostream>
#include <string>
#include <map>
#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#endif
#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/addon/accessibility/fs_taggedpdf.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace foxit::addon::accessibility;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

struct TaggedPdfCommand {
  WString input_file;
  WString output_file;
  bool use_callback;
  bool show_help;
  TaggedPdfCommand()
      : use_callback(false),
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
  ~SdkLibMgr() {
    if (is_initialize_)
      Library::Release();
  }

 private:
  bool is_initialize_;
};

void PrintUsage() {
  cout << "Usage:" << endl;
  cout << "taggedpdf --input <input.pdf> --output <output.pdf> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --input <path>                  Input PDF file path." << endl;
  cout << "  --output <path>                 Output PDF file path." << endl << endl;
  cout << "Optional:" << endl;
  cout << "  --callback                      Use callback for reporting." << endl;
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

bool ParseCommand(int argc, char* argv[], TaggedPdfCommand& command) {
  if (argc <= 1) {
    PrintUsage();
    return false;
  }

  bool has_input = false;
  bool has_output = false;
  for (int i = 1; i < argc; ++i) {
    String key = argv[i];
    if (key.Equal("--help")) {
      command.show_help = true;
      PrintUsage();
      return false;
    }
    if (key.Equal("--callback")) {
      command.use_callback = true;
      continue;
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
      command.output_file = WString::FromUTF8(value);
      has_output = true;
    } else {
      printf("Unknown option: %s\n", (const char*)key);
      PrintUsage();
      return false;
    }
  }

  if (!has_input || !has_output) {
    printf("--input and --output are required.\n");
    PrintUsage();
    return false;
  }
  if (!FileExists(command.input_file)) {
    printf("Input file does not exist: %s\n", (const char*)String::FromUnicode(command.input_file));
    return false;
  }
  return true;
}

static String GetReportCategoryString(TaggedPDFCallback::ReportCategory type) {
  String sFormat = "";
  switch (type) {
  case TaggedPDFCallback::e_ReportCategoryRegion:
    sFormat = "Region";
    break;
  case TaggedPDFCallback::e_ReportCategoryArtifact:
    sFormat = "Artifact";
    break;
  case TaggedPDFCallback::e_ReportCategoryParagraph:
    sFormat = "Paragraph";
    break;
  case TaggedPDFCallback::e_ReportCategoryListItem:
    sFormat = "List Item";
    break;
  case TaggedPDFCallback::e_ReportCategoryFigure:
    sFormat = "Figure";
    break;
  case TaggedPDFCallback::e_ReportCategoryTable:
    sFormat = "Table";
    break;
  case TaggedPDFCallback::e_ReportCategoryTableRow:
    sFormat = "Table Row";
    break;
  case TaggedPDFCallback::e_ReportCategoryTableHeader:
    sFormat = "Table Header";
    break;
  case TaggedPDFCallback::e_ReportCategoryTocItem:
    sFormat = "Toc Item";
    break;
  default:
    break;
  }
  return sFormat;
}

static String GetReportConfidenceString(TaggedPDFCallback::ReportConfidence type) {
  String sFormat = "";
  switch (type) {
  case TaggedPDFCallback::e_ReportConfidenceHigh:
    sFormat = "High";
    break;
  case TaggedPDFCallback::e_ReportConfidenceMediumHigh:
    sFormat = "Medium High";
    break;
  case TaggedPDFCallback::e_ReportConfidenceMedium:
    sFormat = "Medium";
    break;
  case TaggedPDFCallback::e_ReportConfidenceMediumLow:
    sFormat = "Medium Low";
    break;
  case TaggedPDFCallback::e_ReportConfidenceLow:
    sFormat = "Low";
    break;
  default:
    break;
  }
  return sFormat;
}

struct AutoTag_ReportElemRectCon {
  CFX_FloatRect rcRect;
  TaggedPDFCallback::ReportConfidence eConfidence;
};

typedef vector<AutoTag_ReportElemRectCon*> ReportElemRectConArray;
typedef map<TaggedPDFCallback::ReportCategory, ReportElemRectConArray*> ReportElemMap;
typedef map<int, ReportElemMap*> ReportResultPagesMap;
class TaggedPDFCallbackImpl : public TaggedPDFCallback {
public:
  TaggedPDFCallbackImpl() {}

  ~TaggedPDFCallbackImpl() {
    ResetResult();
  }
  virtual void Release() {
    delete this;
  }

  virtual void Report(ReportCategory category, ReportConfidence confidence, int page_index, const RectF& rect) {
    cout << "Page Index: " << page_index << ", ReportCategory: " << (FX_LPCSTR)GetReportCategoryString(category)
      << ", ReportConfidence: " << (FX_LPCSTR)GetReportConfidenceString(confidence) << ", Rect: [" << rect.left
      << ", " << rect.top << ", " << rect.right << ", " << rect.bottom << "]" << endl;

    AutoTag_ReportElemRectCon* pRectCon = new AutoTag_ReportElemRectCon();
    pRectCon->rcRect = rect;
    pRectCon->eConfidence = confidence;

    ReportElemMap* pElemMap = NULL;
    ReportElemRectConArray* pArray = NULL;

    ReportResultPagesMap::iterator it = result_map_.find(page_index);
    if (it == result_map_.end()) {
      pArray = new ReportElemRectConArray();
      pArray->push_back(pRectCon);
      pElemMap = new ReportElemMap();
      pElemMap->insert(ReportElemMap::value_type(category, pArray));
      result_map_.insert(ReportResultPagesMap::value_type(page_index, pElemMap));
    }
    else {
      pElemMap = it->second;
      ReportElemMap::iterator iter = pElemMap->find(category);
      if (iter != pElemMap->end()) {
        pArray = iter->second;
        pArray->push_back(pRectCon);
      }
      else {
        pArray = new ReportElemRectConArray();
        pArray->push_back(pRectCon);
        pElemMap->insert(ReportElemMap::value_type(category, pArray));
      }
    }
  }

  ReportResultPagesMap& GetResult() {
    return result_map_;
  }

  void ResetResult() {
    for (auto iter = result_map_.begin(); iter != result_map_.end(); ++iter) {
      auto item = iter->second;
      for (auto iter_category = item->begin(); iter_category != item->end(); ++iter_category) {
        delete iter_category->second;
      }
      item->clear();
      delete item;
    }
    result_map_.clear();
  }

private:
  ReportResultPagesMap result_map_;
};

int main(int argc, char *argv[])
{
  TaggedPdfCommand command;
  if (!ParseCommand(argc, argv, command)) {
    return 1;
  }

  // Derive output directory from output_file path
  WString output_dir;
  {
    WString output_file = command.output_file;
    int last_sep = -1;
    for (int i = 0; i < (int)output_file.GetLength(); i++) {
      wchar_t ch = output_file.GetAt(i);
      if (ch == L'/' || ch == L'\\') last_sep = i;
    }
    if (last_sep >= 0) {
      output_dir = output_file.Mid(0, last_sep + 1);
    }
  }

#if defined(_WIN32) || defined(_WIN64)
  if (!output_dir.IsEmpty()) _mkdir(String::FromUnicode(output_dir));
#else
  if (!output_dir.IsEmpty()) mkdir(String::FromUnicode(output_dir), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    PDFDoc pdfDoc(command.input_file);
    pdfDoc.Load();
    TaggedPDF taggedpdf(pdfDoc);

    if (command.use_callback) {
      TaggedPDFCallbackImpl* callback = new TaggedPDFCallbackImpl();
      taggedpdf.SetCallback(callback);
    }

    Progressive progressive = taggedpdf.StartTagDocument(NULL);
    Progressive::State progressState = Progressive::e_ToBeContinued;
    while (Progressive::e_ToBeContinued == progressState)
      progressState = progressive.Continue();

    pdfDoc.SaveAs(command.output_file);
    printf("Tagged PDF saved to: %s\n", (const char*)String::FromUnicode(command.output_file));
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

