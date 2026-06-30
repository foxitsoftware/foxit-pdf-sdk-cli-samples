 
// Copyright (C) 2001-2018, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to do OCR for a PDF page or PDF document.

#if !defined(__APPLE__)

// Include Foxit SDK header files.
//#include <time.h>
#include <iostream>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/addon/ocr/fs_ocr.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace addon::ocr;

enum Error_Code {
  Unknow_Error = 100,
  License_Invalid = 101,
  Param_Error = 102,
  GSDK_Init_Error = 103,
  Bin_Path_Error = 104,
  Init_Error = 105,
  Doc_Load_Error = 106,
  Exist_Not_Need_OCR_Page = 129,
} ;

class SdkLibMgr {
public:
  SdkLibMgr() : is_initialize_(false){};
  ErrorCode Initialize(const char* sn, const char* key) {
    ErrorCode error_code = Library::Initialize(sn, key);
    if (error_code != foxit::e_ErrSuccess) {
      printf("Library Initialize Error: %d\n", error_code);
      fflush(stdout);
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

struct ocr_param {
  int type = 0;
  WString ocr_engine_path;
  WString input_file_path;
  WString output_file_path;
  bool is_editable = true;
  WString input_file_password;
  foxit::common::Range page_range = foxit::common::Range();
  int convert_type = 0;
  bool is_flowing_text = true;
  WString languages;
  WString log_file_path;
  bool is_detect_pictures = true;
  bool is_remove_noise = true;
  bool is_correct_skew = true;
  bool is_enable_text_extraction_mode = false;
  bool is_sequentially_process = true;
  bool is_auto_overwrite_resolution = true;
  int resolution_to_overwrite = 300;
  int confidence = 0;
  int ignore_image_width = 0;
  int ignore_image_height = 0;
};

void parse_single_range_string(string range_str, foxit::common::Range& range) {
  int start = -1;
  int end = -1;
  std::size_t pos = range_str.find('-');
  if (pos != string::npos) {
    start = atoi(range_str.substr(0, pos).c_str());
    end = atoi(range_str.substr(pos + 1, range_str.size() - pos).c_str());
    range.AddSegment(start, end);
    //printf("Range: %d - %d, pos=%d\n", start, end, pos);
  } else {
    start = atoi(range_str.c_str());
    range.AddSegment(start, start);
  }
}

void parse_range_string(string range_str, foxit::common::Range& range) {
  std::size_t pos = 0;
  std::size_t len = range_str.length();
  
  while (pos < len) {
    int find_pos = range_str.find(',', pos);
    if (find_pos < 0) {
      parse_single_range_string(range_str.substr(pos, len - pos), range);
      break;
    }
    parse_single_range_string(range_str.substr(pos, find_pos - pos), range);
    pos = find_pos + 1;
  }
}
#define OCRMSG       printf("Please make sure the key %s is valid and it has value.\n", (FX_LPCSTR)argv_key);
#define OCRMSG1            printf("Please try 'ocr_xxx --help' for more information.\n");
bool AnalysisParameter(int argc, char* argv[], struct ocr_param* param) {
  for (int i = 1; i < argc; i = i + 2) {
    String argv_key = String(argv[i]);
    String argv_value;
    if (argc <= i + 1) {
      OCRMSG
      OCRMSG1
      return false;
    }
    argv_value = String(argv[i + 1]);
    if (argv_key.Equal("-type")) param->type = FXSYS_atoi(argv_value);
    else if (argv_key.Equal("-engine")) param->ocr_engine_path = CFX_WideString::FromUTF8(argv_value);
    else if (argv_key.Equal("-input")) param->input_file_path = CFX_WideString::FromUTF8(argv_value);
    else if (argv_key.Equal("-output")) param->output_file_path = CFX_WideString::FromUTF8(argv_value);
    else if (argv_key.Equal("-edit")) param->is_editable = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-pw")) param->input_file_password = CFX_WideString::FromUTF8(argv_value);
    else if (argv_key.Equal("-range")) {
      std::string range_str = argv[i + 1];
      parse_range_string(range_str, param->page_range);
    }
    else if (argv_key.Equal("-format")) param->convert_type = FXSYS_atoi(argv_value);
    else if (argv_key.Equal("-is_retain_flowing_text")) param->is_flowing_text = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-lang")) param->languages = CFX_WideString::FromUTF8(argv_value);
    else if (argv_key.Equal("-log")) param->log_file_path = CFX_WideString::FromUTF8(argv_value);
    else if (argv_key.Equal("-is_detect_pictures"))param->is_detect_pictures = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-is_remove_noise"))param->is_remove_noise = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-is_correct_skew"))param->is_correct_skew = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-is_enable_text_extraction_mode"))param->is_enable_text_extraction_mode = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-is_sequentially_process"))param->is_sequentially_process = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-is_auto_overwrite_resolution"))param->is_auto_overwrite_resolution = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-resolution_to_overwrite"))param->resolution_to_overwrite = FXSYS_atoi(argv_value);
    else if (argv_key.Equal("-confidence"))param->confidence = FXSYS_atoi(argv_value);
    else if (argv_key.Equal("-ignore_image_width"))param->ignore_image_width = FXSYS_atoi(argv_value);
    else if (argv_key.Equal("-ignore_image_height"))param->ignore_image_height = FXSYS_atoi(argv_value);
    else {
      OCRMSG
      OCRMSG1
      return false;
    }
  }

  return true;
}

void output_ocr_param() {
  std::cout << "Please ensure that gsdk_sn.txt, gsdk_key.txt, the Foxit PDF SDK library, and the current executable are all located in the same directory." << std::endl;
  std::cout << "Usage:" << endl <<
    "-type 0 - OCRPDFPage, 1 - OCRPDFDocument, 2 - OCRConvertTo" << endl <<
    "-input The input PDF path." << endl <<
    "-output The output file path." << endl <<
    "-engine The OCR engine path." << endl <<
    "-log The log file path." << endl <<
    "-lang The name of languages which would be included in the language database for doing OCR. For example: 'English, Chinese-Simplified'." << endl <<
    "-edit 'yes' means the OCR result is editable. 'no' means the OCR result can only be searched but not be edited." << endl <<
    "-pw The password of the input PDF file." << endl <<
    "-is_detect_pictures 'yes' or 'no'. Decide whether to detect pictures." << endl <<
    "-is_remove_noise 'yes' or 'no'. Decide whether to remove noise of the image of PDF." << endl <<
    "-is_correct_skew 'yes' or 'no'. Decide whether to enable skew correction." << endl <<
    "-is_enable_text_extraction_mode 'yes' or 'no'. Decide whether to enable text extraction mode." << endl <<
    "-is_sequentially_process 'yes' or 'no'. Decide whether the OCR engine will process pages sequentially on one process. Default: 'yes'." << endl <<
    "-is_auto_overwrite_resolution 'yes' or 'no'. Decide whether to auto overwrite resolution. Default: 'yes'." << endl <<
    "-resolution_to_overwrite The resolution to overwrite. This parameter is valid only when parameter 'is_auto_overwrite_resolution' is set to 'no'." << endl <<
    "-confidence The confidence threshold used to determine whether the recognized text is reliable. The value range is from 0 to 100. Default: '0'." << endl <<
    "-range The range of pages. This parameter is valid when 'type' is 0 or 2.  For example: '0,3-5,7'. If this parameter is not set, all the page index will be set." << endl <<
    "-format The format of the document to convert. This parameter is valid when 'type' is 2. Valid value:[0-6], 0 - DOCX, 1 - DOC, 2 - RTF, 3 - XLSX, 4 - XLS, 5 - PPTX, 6 - HTML. Default: '0'." << endl <<
    "-is_retain_flowing_text 'yes' means the generated document will retain flowing text. 'no' means the generated document will retain original page layout. This parameter is valid when 'format' is [0-2].  Default: 'yes'." << endl <<
    "-ignore_image_width Ignore images with width less than this value during OCR processing. This parameter is valid when 'type' is 0 or 1. Parameter 'ignore_image_height' needs to be set simultaneously. Default: '0'. " << endl <<
    "-ignore_image_height Ignore images with height less than this value during OCR processing. This parameter is valid when 'type' is 0 or 1. Parameter 'ignore_image_width' needs to be set simultaneously. Default: '0'." << endl;
}

bool check_ocr_param(const struct ocr_param* param) {
  if (param->type < 0 || param->type > 3)
    return false;
  
  if (param->input_file_path.GetLength() == 0 || param->output_file_path.GetLength() == 0)
    return false;
  
  if (param->type == 2) {
    if (param->convert_type < 0 || param->convert_type > 6)
      return false;
  }

  return true;
}

class MyOCRCallback : public OCRCallback {
public:
  MyOCRCallback(int w, int h) : width_(w), height_(h) {
  }

  virtual bool NeedToCancelNow(const wchar_t* error) {
    return false;
  }
  virtual bool IsImageIgnored(pdf::graphics::ImageObject* image_object) {
    RectF rect = image_object->GetRect();
    if (rect.Width() < width_ && rect.Height() < height_) {
      return true;
    }
    return false;
  }

private:
  int width_;
  int height_;
};

class MyOCRProgressCallback : public OCRProgressCallback {
public:
  MyOCRProgressCallback() {
  };
  ~MyOCRProgressCallback() {};
  virtual void ProgressNotify(int current_rate) {
    printf("progress:[%d%%]\n", current_rate);
    fflush(stdout);
  }
};

int main(int argc, char *argv[]) {
  int err_ret = 0;
  if (argc > 1 && String(argv[1]).Equal("--help")) {
    output_ocr_param();
    return 0;
  }
  struct ocr_param ocr_param;
  AnalysisParameter(argc, argv, &ocr_param);

  //output_ocr_param(&ocr_param);

  if (!check_ocr_param(&ocr_param)) {
    OCRMSG1
    return Error_Code::Param_Error;
  }
  
  static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
  static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize(sn, key);
  if (error_code != foxit::e_ErrSuccess) {
    return Error_Code::GSDK_Init_Error;
  }

  try {
    // "ocr_resource_path" is the path of ocr resources. Please refer to Developer Guide for more details.
    WString ocr_resource_path = ocr_param.ocr_engine_path;

    if (ocr_resource_path.IsEmpty()) {
      printf("ocr_resource_path is still empty. Please set it with a valid path to OCR resource path.\n");
      fflush(stdout);
      return Error_Code::Bin_Path_Error;
    }

    // Initialize OCR engine.
    error_code = OCREngine::Initialize(ocr_resource_path);
    if (error_code != foxit::e_ErrSuccess) {
      switch(error_code){
        case foxit::e_ErrInvalidLicense:
          printf("[Failed] OCR module is not contained in current Foxit PDF SDK keys.\n");
          fflush(stdout);
          break;
        default:
          printf("Fail to initialize OCR engine. Error: %d\n", error_code);
          fflush(stdout);
          break;
      }
      return Error_Code::Init_Error;
    }
    printf("OCREngine is initialized.\n");
    fflush(stdout);
    if (ocr_param.log_file_path.GetLength() > 0)
      OCREngine::SetLogFile(ocr_param.log_file_path);

    if (ocr_param.languages.GetLength() > 0)
      OCREngine::SetLanguages(ocr_param.languages);
  
    MyOCRCallback ocr_callback(ocr_param.ignore_image_width, ocr_param.ignore_image_height);
    MyOCRProgressCallback ocr_progress_callback;
    OCREngine::SetOCRCallback(&ocr_callback);
    OCRConfig ocr_config;
    ocr_config.is_detect_pictures = ocr_param.is_detect_pictures;
    ocr_config.is_remove_noise = ocr_param.is_remove_noise;
    ocr_config.is_correct_skew = ocr_param.is_correct_skew;
    ocr_config.is_enable_text_extraction_mode = ocr_param.is_enable_text_extraction_mode;
    ocr_config.is_sequentially_process = ocr_param.is_sequentially_process;
    ocr_config.is_auto_overwrite_resolution = ocr_param.is_auto_overwrite_resolution;
    ocr_config.resolution_to_overwrite = ocr_param.resolution_to_overwrite;
    ocr_config.confidence = ocr_param.confidence;
    if (ocr_param.type == 0)
    {
      PDFDoc doc(ocr_param.input_file_path);
      ErrorCode error_code = doc.LoadW(ocr_param.input_file_password);
      if (error_code != foxit::e_ErrSuccess)
      {
        printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(ocr_param.input_file_path), error_code);
        return Error_Code::Doc_Load_Error;
      }
      
      OCR ocr_operator;
      int page_count = doc.GetPageCount();
      if (ocr_param.page_range.IsEmpty()) {
        ocr_param.page_range.AddSegment(0, page_count - 1);
      }
      int segment_count = ocr_param.page_range.GetSegmentCount();
      for (int i = 0; i < segment_count; i++) {
        int start_index = 0;
        int end_index = 0;
        start_index = ocr_param.page_range.GetSegmentStart(i);
        end_index = ocr_param.page_range.GetSegmentEnd(i);
        if (start_index > page_count - 1)
          start_index = 0;
        if (end_index > page_count - 1)
          end_index = page_count - 1;
        for (int j = start_index; j <= end_index; j++) {
          PDFPage page = doc.GetPage(j);
          page.StartParse();
          ocr_operator.OCRPDFPage(page, ocr_param.is_editable, ocr_config, &ocr_progress_callback);
        }
      }

      doc.SaveAs(ocr_param.output_file_path, PDFDoc::e_SaveFlagNormal);
    }
    else if (ocr_param.type == 1) {
      PDFDoc doc(ocr_param.input_file_path);
      ErrorCode error_code = doc.LoadW(ocr_param.input_file_password);
      if (error_code != foxit::e_ErrSuccess)
      {
        printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(ocr_param.input_file_path), error_code);
        return Error_Code::Doc_Load_Error;
      }
      OCR ocr_operator;
      ocr_operator.OCRPDFDocument(doc, ocr_param.is_editable, ocr_config, &ocr_progress_callback);

      doc.SaveAs(ocr_param.output_file_path, PDFDoc::e_SaveFlagNormal);
    }
    else if (ocr_param.type == 2) {
      OCR ocr_operator;
      ocr_operator.OCRConvertTo((OCR::OCRConvertFormat)ocr_param.convert_type, ocr_param.input_file_path,ocr_param.input_file_password,ocr_param.output_file_path, ocr_param.page_range, ocr_param.is_flowing_text, ocr_config, &ocr_progress_callback);
    }
    
    OCREngine::Release();

    cout << "END: OCR demo." << endl;
  } catch (const Exception& e) {
    switch(e.GetErrCode()){
      case foxit::e_ErrInvalidLicense:
        printf("[Failed] OCR module is not contained in current Foxit PDF SDK keys.\n");
        err_ret = Error_Code::License_Invalid;
        break;
      default:
        printf("%s\n", (const char*)e.GetMessage());
        err_ret = Error_Code::Unknow_Error;
        break;
    }
  }
  catch(const std::exception& e){
    cout << e.what() << endl;
  }
  catch(...)
  {
    cout << "Unknown Exception" << endl;
  }

  return err_ret;
}
#endif  // #if !defined(__APPLE__)

