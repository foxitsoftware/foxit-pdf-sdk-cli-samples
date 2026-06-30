// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to call layout recognition related classes
// for PDF document.

// Include Foxit SDK header files.
#include <time.h>
#include <iostream>

#define _FSDK_LAYOUTRECOGNITION_

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/addon/layoutrecognition/fs_layoutrecognition.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace annots;
using namespace actions;
using namespace interform;
using namespace foxit::addon::layoutrecognition;
using namespace foxit::pdf::objects;
using namespace foxit::pdf::graphics;

void PrintUsage() {
  cout << "Usage:" << endl
       << "layout_recognition <input pdf path> <output info path>" << endl
       << endl
       << "<input pdf path>  The input pdf path." << endl
       << "<output info path>  The output info file path." << endl;
}

bool AnalysisParameter(int argc, char* argv[], WString& input_file, WString& output_file) {
  if (argc != 3) {
    PrintUsage();
    return false;
  }

  input_file = WString::FromUTF8(String(argv[1]));
  output_file = WString::FromUTF8(String(argv[2]));
  if (input_file.IsEmpty() || output_file.IsEmpty()) {
    PrintUsage();
    return false;
  }

  return true;
}

static const char* sn = "VpSo5y1BvObesk1dr3z6TraG0CqNePvDKOsnMiFGAhjVHxrkdjxVeA==";
static const char* key = "8f0YFUFsvWkN+Td6AF43wjkod4jYNBz9a0B2/doz2qcBlc7sqAyBnS41P1KCTKZ/um0szmI0GNgFVYLxSwrKjzS7+91xsmiP+a3osElOj0d49EfWrxHE2BZx/zyAn/gsgFvDkXjTpd/GVO7QNKoSJDohajahakh+bXlLaT1fNgTPzdQ+39W/XbBbWh4cgVUw5cm0DH8LBmQXxgiWCmWCLP/tC/gWZqenyXKMusYr6UPmzuFN3uPfIdq3fup3hilLjl6HBSPPHB4DLB4MIztrTlt7xK/pdF9VYw/Jmltqc/e859Ov8RgftPzCjOAD6gkfNKSi50XHJLO5Z+aoC89SYhxQ3cueRNHEYOz+P3lD4j3KjGcTyUFiCA6eP/VjN/dE56vlUsTXhhlVWqw1tZkB7kJh4RWEaXtEf70F4DZzDf+V3ZrklfI0Tr5xrRZCE2Lw4+R8VTszwQaIke2XgyXn17lM6/tSnHboZUJRnX2BMr8c5Bc/PoSEUnSUeenGd91kI/zUmqInGmxVADAeGgu19vLegUNM/HkItHlAl5AiZVKoM4AyEsa6SpC4369v698y3RGPZcM7p4aJJKco7tV+aqLXqM7maw/9ll4VzAlIckR0QPniPieGmDk1D4oqsg/wI/6RZhYSLukxANELAOoqNPGx3wTbkObNsI9EAfKWJS05FtB4WXp2UCA/sW3iGtXXPq+WOSdO8Z+SD4p/QNe9NoIn7TO2s8FcVioubKiVCkazXBwUv5kxyPUqUlo5M4ja5ByATKKw6/twkkt3Or6UnfLgszoNY4v03PonQTAIvrKgq5TuC9UcFub5UlbxGivxnde8YmXxlBBzcwjzXSmcq0CHSJ9SHvyaJlbN1tlmllomCQqq5A7VryHq/IQ759qdxEa96oalvK9912nMZD+FEo9aZiYp+a058TpoZl3sKSHXvpwl1FSJiZpScoQ6nv2cGOTo3o2xgjR99sC2GCRg8X1WwegAajCCdZwvg8g/EbcYTuqUgFmXX8oj0iVKWD5i6Wl5FJ3qP3AdUxpI19oz/T8owMi9UGI6UKH2qYlESUSFTf3lmELjYbvmr5kpeWzikG8++APGC/+2o4m2pW+mT7ocPmRNvTW/W2BAdxuFI9Gf87hItt607X2Q1mub9c+8BgUl4BiyERIa4yY11RHhlEVkvw0vxgOuttX5ZCgmkZTIHdcd6XXSBq5pl7Tj1J3QjT9dKzLK4b+IPrQ1UTFWmHi53n2iqBl99aYIFd78nZCz8VScmHlNPztVWPQWKM4qUoqtY+legw==";

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

class TextDoc {
 public:
  TextDoc(const String& file_name, const String& fill_mode);
  TextDoc(const WString& file_name, const WString& fill_mode);
  ~TextDoc();

  void Write(const char* text_content);
  void Write(const wchar_t* text_content);
  void Write(int count, const wchar_t* prefix, const wchar_t* text_content);

 private:
  FILE* file_;
};

TextDoc::TextDoc(const String& file_name, const String& file_mode) throw(Exception)
    : file_(NULL) {
#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file_, file_name, (const char*)file_mode);
#else
  file_ = fopen((const char*)file_name, (const char*)file_mode);
#endif

  if (!file_)
    throw Exception(__FILE__, __LINE__, __FUNCTION__, foxit::e_ErrFile);

  uint8 temp[] = {0xFF, 0xFE};
  fwrite(temp, sizeof(uint8), 2, file_);
  fseek(file_, 0, SEEK_END);
}

TextDoc::TextDoc(const WString& file_name, const WString& file_mode) throw(Exception)
    : file_(NULL) {
  String s_file_name = String::FromUnicode(file_name);
  String s_file_mode = String::FromUnicode(file_mode);

#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file_, (const char*)s_file_name, (const char*)s_file_mode);
#else
  file_ = fopen((const char*)s_file_name, (const char*)s_file_mode);
#endif
  if (!file_)
    throw Exception(__FILE__, __LINE__, __FUNCTION__, foxit::e_ErrFile);

  uint8 temp[] = {0xFF, 0xFE};
  fwrite(temp, sizeof(uint8), 2, file_);
  fseek(file_, 0, SEEK_END);
}

TextDoc::~TextDoc() {
  fclose(file_);
  file_ = NULL;
}

void TextDoc::Write(const char* text_content) {
  WString wide_str = WString::FromLocal(text_content);
  Write(wide_str);
}

void TextDoc::Write(const wchar_t* text_content) {
  WString wide_str(text_content);
  if (wide_str.IsEmpty())
    return;
  String utf16le_str = wide_str.UTF16LE_Encode(false);
  if (utf16le_str.IsEmpty())
    return;
  int length = utf16le_str.GetLength();
  fwrite((const char*)utf16le_str, sizeof(char), length, file_);
}

void TextDoc::Write(int count, const wchar_t* prefix, const wchar_t* text_content) {
  for (int i = 0; i < count; i++) {
    Write(prefix);
  }

  Write(text_content);
}

typedef CFX_ArrayTemplate<LRElement> LRElementArray;

void GetChildFromElement(LRStructureElement pElement, LRElementArray& elemArray,
                         LRStructureElement::ElementType nType) {
  int nElemListSize = pElement.GetChildCount();
  for (int i = 0; i < nElemListSize; i++) {
    LRElement item = pElement.GetChild(i);
    LRElement::ElementType item_type = item.GetElementType();
    if (item_type == nType)
      elemArray.Add(item);
    if (item.IsStructureElement()) {
      LRStructureElement srt_item(item);
      GetChildFromElement(srt_item, elemArray, nType);
    }
  }
}

WString LR_Format(bool val) {
  return val ? L"True" : L"False";
}

WString LR_Format(int32 val) {
  WString s;
  s.Format(L"%d", val);
  return s;
}

WString LR_Format(float val) {
  WString s;
  s.Format(L"%.1f", val);
  return s;
}

WString LR_Format(WString val) {
  return val;
}

WString LR_Format(ARGB val) {
  WString s;
  s.Format(L"#%02X%02X%02X", (uint8)(val >> 16), (uint8)(val >> 8), (uint8)(val));
  return s;
}

WString LR_Format(LRStructureElement::AttributeValueEnum val) {
  return WString::FromLocal(LRStructureElement::StringifyEnumVariant(val));
}

void OutputLRStructureElement(LRStructureElement element, TextDoc& doc, int depth) {
  if (depth > 32)
    return;
  if (element.IsEmpty())
    return;

  if (element.IsEmpty())
    return;

  LRStructureElement parentElement = element.GetParentElement();
  bool bIsEmpty = parentElement.IsEmpty();

  LRStructureElement::ElementType elemType = element.GetElementType();
  String elementTypeStr = LRElement::StringifyElementType(elemType);
  WString outputStr = L"< StructureElement: " + WString::FromLocal(elementTypeStr) + L" >\r\n";
  doc.Write(depth, L"\t", outputStr);
  int32 nSize = element.GetSupportedAttributeCount();
  for (int i = 0; i < nSize; i++) {
    LRStructureElement::AttributeType attrType = element.GetSupportedAttribute(i);

    LRStructureElement::AttributeValueType attrValueType = element.GetAttributeValueType(attrType);
    int nLength = 0;
    bool bIsArray = LRStructureElement::IsArrayAttributeValueType(attrValueType);
    nLength = element.GetAttributeValueCount(attrType);
    if (attrValueType == LRStructureElement::e_AttributeValueTypeEmpty)
      continue;

    WString szKey = WString::FromLocal(LRStructureElement::StringifyAttributeType(attrType));
    WString szVal = L"";
    if (bIsArray)
      szVal += L"[";

    for (int idx = 0; idx < nLength; idx++) {
      if (idx)
        szVal += L", ";

      switch (attrValueType) {
        case LRStructureElement::e_AttributeValueTypeEnum:
        case LRStructureElement::e_AttributeValueTypeEnumArray:
          szVal += LR_Format(element.GetAttributeValueEnum(attrType, idx));
          break;
        case LRStructureElement::e_AttributeValueTypeInt32:
        case LRStructureElement::e_AttributeValueTypeInt32Array:
          szVal += LR_Format(element.GetAttributeValueInt32(attrType, idx));
          break;
        case LRStructureElement::e_AttributeValueTypeFloat:
        case LRStructureElement::e_AttributeValueTypeFloatArray:
          szVal += LR_Format(element.GetAttributeValueFloat(attrType, idx));
          break;
        case LRStructureElement::e_AttributeValueTypeARGB:
        case LRStructureElement::e_AttributeValueTypeARGBArray:
          szVal += LR_Format(element.GetAttributeValueARGB(attrType, idx));
          break;
        case LRStructureElement::e_AttributeValueTypeWStr:
        case LRStructureElement::e_AttributeValueTypeWStrArray:
          szVal += LR_Format(element.GetAttributeValueString(attrType, idx));
          break;
        default:
          break;
      }
    }
    if (bIsArray)
      szVal += L"]";

    outputStr = szKey + L": " + szVal + L"\r\n";
    doc.Write(depth, L"\t", outputStr);
  }
}

void OutputLRStructureElement(LRGraphicsObjectElement element, TextDoc& doc, int depth) {
  if (element.IsEmpty())
    return;

  WString outputStr = L"< LRGraphicsObjectElement: >\r\n";
  LRGraphicsObjectElement parentPageObj = element.GetParentGraphicsObjectElement();
  bool bIsEmpty = parentPageObj.IsEmpty();
  doc.Write(depth, L"\t", outputStr);
  GraphicsObject* pageObj = element.GetGraphicsObject();
  GraphicsObject::Type type = GraphicsObject::e_TypeAll;
  if (pageObj != NULL)
    type = pageObj->GetType();
  RectF rcBox = element.GetBBox();
  PDFDictionary* pDict = element.GetDict();
  int objIndex = element.GetGraphicsObjectIndex();
  Matrix matrix = element.GetMatrix();
  WString szVal = L"";
  szVal.Format(L"BBox: [%.1f,%.1f,%.1f,%.1f]\r\n", rcBox.left, rcBox.top, rcBox.right, rcBox.bottom);
  doc.Write(depth, L"\t", szVal);
  szVal.Format(L"Matrix: [%.1f,%.1f,%.1f,%.1f,%.1f,%.1f]\r\n", matrix.a, matrix.b, matrix.c, matrix.d, matrix.e,
               matrix.f);
  doc.Write(depth, L"\t", szVal);
  szVal.Format(L"PageObjectIndex: %d\r\n", objIndex);
  doc.Write(depth, L"\t", szVal);

  GraphicsObject* pObject = element.GetGraphicsObject();
  WString text = (pObject && pObject->GetType() == GraphicsObject::e_TypeText)
                     ? element.GetGraphicsObject()->GetTextObject()->GetText()
                     : L"";
  if (!text.IsEmpty()) {
    szVal = L"Text: ";
    szVal += text;
    szVal += L"\r\n";

    doc.Write(depth, L"\t", szVal);
  }
}

void OutputLRStructureElement(LRContentElement element, TextDoc& doc, int depth) {
  if (element.IsEmpty())
    return;

  LRGraphicsObjectElement pageObj = element.GetGraphicsObjectElement();
  bool bIsEmpty = pageObj.IsEmpty();
  LRStructureElement parentPageObj = element.GetParentElement();
  bIsEmpty = parentPageObj.IsEmpty();

  LRElement::ElementType elemType = element.GetElementType();
  String elementTypeStr = LRStructureElement::StringifyElementType(elemType);
  WString outputStr = L"< LRContentElement: " + WString::FromLocal(elementTypeStr) + L" >\r\n";
  doc.Write(depth, L"\t", outputStr);

  RectF rcBox = element.GetBBox();
  int32 nStartPos = 0, nLength = 0;
  common::Range range = element.GetGraphicsObjectRange();
  if (!range.IsEmpty()) {
    nStartPos = range.GetSegmentStart(0);
    nLength = range.GetSegmentEnd(0) - nStartPos + 1;
  }
  Matrix matrix = element.GetMatrix();

  WString szVal = L"";
  szVal.Format(L"BBox: [%.1f,%.1f,%.1f,%.1f]\r\n", rcBox.left, rcBox.top, rcBox.right, rcBox.bottom);
  doc.Write(depth, L"\t", szVal);
  szVal.Format(L"Matrix: [%.1f,%.1f,%.1f,%.1f,%.1f,%.1f]\r\n", matrix.a, matrix.b, matrix.c, matrix.d, matrix.e,
               matrix.f);
  doc.Write(depth, L"\t", szVal);
  szVal.Format(L"StartPos: %d\r\n", nStartPos);
  doc.Write(depth, L"\t", szVal);
  szVal.Format(L"Length: %d\r\n", nLength);
  doc.Write(depth, L"\t", szVal);

  if (!bIsEmpty)
    OutputLRStructureElement(pageObj, doc, depth + 1);
}

void ShowLRElementInfo(LRStructureElement element, TextDoc& doc, int depth) {
  OutputLRStructureElement(element, doc, depth);
  int nElemListSize = element.GetChildCount();
  for (int i = 0; i < nElemListSize; i++) {
    LRElement item = element.GetChild(i);
    uint32 item_type = item.GetElementType();
    if (item.IsStructureElement()) {
      LRStructureElement srt_item(item);
      ShowLRElementInfo(srt_item, doc, depth + 1);
    } else if (item.IsContentElement()) {
      LRContentElement srt_item(item);
      OutputLRStructureElement(srt_item, doc, depth);
    } else if (item.IsGraphicsObjectElement()) {
      LRGraphicsObjectElement srt_item(item);
      OutputLRStructureElement(srt_item, doc, depth);
    }
  }
}

void OutputAllLRElement(LRStructureElement element, const WString& info_path) {
  TextDoc text_doc(info_path, L"w+b");
  if (!element.IsEmpty()) {
    ShowLRElementInfo(element, text_doc, 0);
  } else {
    text_doc.Write("No layout recognition information!\r\n");
  }
}

int main(int argc, char* argv[]) {
  if (argc > 1 && String(argv[1]).Equal("--help")) {
    PrintUsage();
    return 0;
  }

  WString input_file;
  WString output_file;
  if (!AnalysisParameter(argc, argv, input_file, output_file)) return 1;

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    WString info_file = output_file;

    PDFDoc doc(input_file);
    ErrorCode error_code = doc.Load();
    if (error_code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(input_file), error_code);
      return 1;
    }

    LRContext context(doc.GetPage(0));

    Progressive progressive = context.StartParse(NULL);

    LRStructureElement root = context.GetRootElement();
    LRStructureElement::ElementType type = root.GetElementType();
    int nElemListSize = root.GetChildCount();
    cout << "RootElement GetChildCount: " << nElemListSize << endl;

    OutputAllLRElement(root, info_file);

    LRElementArray element_array;
    GetChildFromElement(root, element_array, LRStructureElement::e_ElementTypeSpan);
    cout << "LRElement::e_ElementTypeSpan count: " << element_array.GetSize() << endl;

    cout << "Layout recognition test." << endl;
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  } catch (...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}
