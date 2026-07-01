// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to convert images to PDF files.

// Include Foxit SDK header files.
#include <time.h>
#include <iostream>
#include <vector>
#include <algorithm>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/common/fs_image.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/graphics/fs_pdfgraphicsobject.h"
#include "addon/conversion/fs_convert.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace pdf::graphics;
using namespace foxit::addon::conversion;

static const char* sn = "";
static const char* key = "";

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

#define IMAGE2PDFMSG       printf("Please make sure the key %s is valid and it has value.\n", (FX_LPCSTR)argv_key); \
                          printf("Usage: image2pdf_xxx -o <output pdf path> -p <page>:<x>:<y>:<input image path> [-p ...]\nPlease try 'image2pdf_xxx --help' for more information.\n");

struct PageRule {
  int page_index;
  int pos_x;
  int pos_y;
  WString image_path;
};

bool ParseIntValue(const String& value, int& result) {
  std::string text = std::string((const char*)value);
  char* end_ptr = NULL;
  long parsed = strtol(text.c_str(), &end_ptr, 10);
  if (end_ptr == text.c_str() || *end_ptr != '\0')
    return false;
  result = (int)parsed;
  return true;
}

bool FileExists(const WString& path) {
  FILE* file = NULL;
#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file, String::FromUnicode(path), "rb");
#else
  file = fopen(String::FromUnicode(path), "rb");
#endif
  if (file == NULL)
    return false;
  fclose(file);
  return true;
}

bool ParsePageRule(const String& value, PageRule& rule) {
  std::string text = std::string((const char*)value);
  size_t first = text.find(':');
  size_t second = (first == std::string::npos) ? std::string::npos : text.find(':', first + 1);
  size_t third = (second == std::string::npos) ? std::string::npos : text.find(':', second + 1);
  if (first == std::string::npos || second == std::string::npos || third == std::string::npos)
    return false;

  String page_value = String(text.substr(0, first).c_str());
  String x_value = String(text.substr(first + 1, second - first - 1).c_str());
  String y_value = String(text.substr(second + 1, third - second - 1).c_str());
  String image_value = String(text.substr(third + 1).c_str());
  if (image_value.IsEmpty())
    return false;

  if (!ParseIntValue(page_value, rule.page_index) || rule.page_index <= 0)
    return false;
  if (!ParseIntValue(x_value, rule.pos_x) || rule.pos_x < 0)
    return false;
  if (!ParseIntValue(y_value, rule.pos_y) || rule.pos_y < 0)
    return false;
  rule.image_path = WString::FromUTF8(image_value);
  return true;
}

bool AnalysisParameter(int argc, char* argv[], WString& output_file, std::vector<PageRule>& page_rules, bool& show_help) {
  show_help = false;
  output_file = L"";
  page_rules.clear();

  if (argc == 2 && String(argv[1]).Equal("--help")) {
    show_help = true;
    return true;
  }

  for (int i = 1; i < argc; i = i + 2) {
    String argv_key = String(argv[i]);
    String argv_value;
    if (argc <= i + 1) {
      IMAGE2PDFMSG
      return false;
    }
    argv_value = String(argv[i + 1]);
    if (argv_key.Equal("-o")) {
      output_file = WString::FromUTF8(argv_value);
    } else if (argv_key.Equal("-p")) {
      PageRule rule;
      if (!ParsePageRule(argv_value, rule)) {
        cout << "Invalid page rule: " << (FX_LPCSTR)argv_value << endl;
        return false;
      }
      if (!FileExists(rule.image_path)) {
        cout << "Image file does not exist: " << (FX_LPCSTR)String::FromUnicode(rule.image_path) << endl;
        return false;
      }
      for (size_t index = 0; index < page_rules.size(); ++index) {
        if (page_rules[index].page_index == rule.page_index) {
          cout << "Duplicate output page index: " << rule.page_index << endl;
          return false;
        }
      }
      page_rules.push_back(rule);
    } else {
      IMAGE2PDFMSG
      return false;
    }
  }

  if (output_file.IsEmpty() || page_rules.empty()) {
    cout << "Missing required parameter(s): -o and at least one -p are required." << endl;
    return false;
  }

  return true;
}

void Image2PDF(const std::vector<PageRule>& page_rules, WString output_file)
{
  std::vector<PageRule> sorted_rules = page_rules;
  std::sort(sorted_rules.begin(), sorted_rules.end(), [](const PageRule& left, const PageRule& right) {
    return left.page_index < right.page_index;
  });

  PDFDoc doc;
  for (size_t rule_index = 0; rule_index < sorted_rules.size(); ++rule_index) {
    const PageRule& rule = sorted_rules[rule_index];
    Image image(rule.image_path);
    FX_FLOAT page_width = image.GetWidth() + rule.pos_x;
    FX_FLOAT page_height = image.GetHeight() + rule.pos_y;
    PDFPage page = doc.InsertPage((int)rule_index, page_width, page_height);
    page.StartParse(foxit::pdf::PDFPage::e_ParsePageNormal, NULL, false);
    page.AddImage(image, 0, PointF((FX_FLOAT)rule.pos_x, (FX_FLOAT)rule.pos_y), image.GetWidth(), image.GetHeight(), true);
  }
  doc.SaveAs(output_file, PDFDoc::e_SaveFlagNoOriginal);

}

int main(int argc, char *argv[])
{
  int err_ret = 0;
  WString output_file;
  std::vector<PageRule> page_rules;
  bool show_help = false;
  if (!AnalysisParameter(argc, argv, output_file, page_rules, show_help)) {
    cout << "Usage:" << endl << "image2pdf_xxx -o <output pdf path> -p <page>:<x>:<y>:<input image path> [-p ...]" << endl << endl
         << "-o The output pdf path." << endl
         << "-p <page>:<x>:<y>:<input image path>  The output page index, insertion position and image path." << endl;
    return 1;
  }
  if (show_help) {
    cout << "Usage:" << endl << "image2pdf_xxx -o <output pdf path> -p <page>:<x>:<y>:<input image path> [-p ...]" << endl << endl
         << "-o The output pdf path." << endl
         << "-p <page>:<x>:<y>:<input image path>  The output page index, insertion position and image path." << endl;
    return 0;
  }

  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    Image2PDF(page_rules, output_file);
    cout << "Convert image file to PDF file." << endl;
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  }

  return err_ret;
}

