// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to mark PDF content and
// get marked content from graphics objects.

// Include Foxit SDK header files.
#include <time.h>
#include <iostream>

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/common/fs_image.h"
#include "../../../include/pdf/graphics/fs_pdfgraphicsobject.h"
#include "../../../include/pdf/objects/fs_pdfobject.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace graphics;
using namespace objects;

void PrintUsage() {
  cout << "Usage:" << endl << "marked_content <output pdf path>" << endl << endl <<
      "output pdf path: The output pdf path." << endl;
}

bool AnalysisParameter(int argc, char* argv[], WString& input_file, WString& output_file) {
  if (argc != 3) {
    return false;
  }

  input_file = WString::FromUTF8(String(argv[1]));
  output_file = WString::FromUTF8(String(argv[2]));
  return true;
}

static const char* sn = "";
static const char* key = "";

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

void CreateMarkedContent(PDFPage page) {
  POSITION position = page.GetLastGraphicsObjectPosition(GraphicsObject::e_TypeText);
  TextObject* text_object = TextObject::Create();

  text_object->SetFillColor(0xFF83AF9B);

  // Prepare text state
  TextState state;
  state.font_size = 72.f;
  state.font = Font(Font::e_StdIDTimesB);
  state.textmode = TextState::e_ModeFill;

  text_object->SetTextState(page, state, false, 750);

  text_object->SetText(L"Marked-Content");
  page.InsertGraphicsObject(position, text_object);

  RectF rect = text_object->GetRect();
  float offset_x = (page.GetWidth() - (rect.right - rect.left)) / 2;
  float offset_y = page.GetHeight() * 0.618f - (rect.top - rect.bottom) / 2;
  text_object->Transform(Matrix(1, 0, 0, 1, offset_x, offset_y), false);

  MarkedContent* content = text_object->GetMarkedContent();
  PDFDictionary* span_dict = PDFDictionary::Create();
  span_dict->SetAtInteger("Direct", 55);
  content->AddItem("Span",span_dict);
  span_dict->Release();
  // Generate content
  page.GenerateContent();
}

bool GetMarkedContentObject(WString pdf_file) {
  PDFDoc doc(pdf_file);
  ErrorCode ret = doc.Load();
  if (ret != foxit::e_ErrSuccess) {
    printf("[Failed] Cannot load PDF document %s.\r\n Error message:%d\r\n", (const char*)String::FromUnicode(pdf_file), ret);
    return false;
  }
  PDFPage page = doc.GetPage(0);
  page.StartParse(foxit::pdf::PDFPage::e_ParsePageNormal, NULL, false);
  POSITION position = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeText);
  TextObject* text_obj = reinterpret_cast<TextObject*>(page.GetGraphicsObject(position));
  MarkedContent* content = text_obj->GetMarkedContent();
  int item_count = content->GetItemCount();

  WString file_info = pdf_file + L".markedcontent.txt";
  TextDoc text_doc(file_info, L"w+b");
  // Get marked content property
  for (int i = 0; i < item_count; i++) {
    text_doc.Write("index: %d\r\n", i);
    String tag_name = content->GetItemTagName(i);
    text_doc.Write("Tag name: %s\r\n", (const char*)tag_name);
    int mcid = content->GetItemMCID(i);
    text_doc.Write("mcid: %d\r\n", mcid);
  }
  return true;
}

int main(int argc, char *argv[])
{
  if ((argc > 1 && String(argv[1]).Equal("--help")) || argc < 2) {
    PrintUsage();
    return 0;
  }

  WString input_file;
  WString output_file;
  if (!AnalysisParameter(argc, argv, input_file, output_file)) {
    PrintUsage();
    return 1;
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    PDFDoc doc;
    PDFPage page = doc.InsertPage(0);

    CreateMarkedContent(page);

    doc.SaveAs(output_file, PDFDoc::e_SaveFlagNormal);

    if (!GetMarkedContentObject(output_file)) {
      return 1;
    }
    cout << "Add marked content to PDF file." << endl;

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
