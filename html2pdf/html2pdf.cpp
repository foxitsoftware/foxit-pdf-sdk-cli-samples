// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to convert url web pages or html files to PDF document.
// If using a stream to load the html file, set variable isfilestreamload to true.
// Include Foxit SDK header files.
#if (defined(_WIN32) || defined(_WIN64) || defined (__APPLE__) || defined (__linux__)) && !defined(__arm__)

#include<string.h>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/addon/conversion/fs_convert.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace common::file;

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
  SdkLibMgr() : is_initialize_(false) {};
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

#define HTML2PDFMSG       printf("Please make sure the key %s is valid and it has value.\n", (FX_LPCSTR)argv_key); \
                          printf("Usage: html2pdf_xxx <-html <The url or html path>> <-o <output pdf path>> ...\nPlease try 'html2pdf_xxx --help' for more information.\n");

bool AnalysisParameter(int argc, char* argv[], foxit::addon::conversion::HTML2PDFSettingData& pdf_setting_data, WString& url_or_html, WString& engine_path, WString& cookies_path, WString& output_path, int& timeout) {
  for (int i = 1; i < argc; i = i + 2) {
    String argv_key = String(argv[i]);
    String argv_value;
    if (argc <= i + 1) {
      HTML2PDFMSG
      return false;
    }
    argv_value = String(argv[i + 1]);
    if (argv_key.Equal("-w")) pdf_setting_data.page_width = FX_atof(argv_value);
    else if (argv_key.Equal("-h")) pdf_setting_data.page_height = FX_atof(argv_value);
    else if (argv_key.Equal("-r")) pdf_setting_data.rotate_degrees = (foxit::common::Rotation)FXSYS_atoi(argv_value);
    else if (argv_key.Equal("-ml")) pdf_setting_data.page_margin.left = FX_atof(argv_value);
    else if (argv_key.Equal("-mr")) pdf_setting_data.page_margin.right = FX_atof(argv_value);
    else if (argv_key.Equal("-mt")) pdf_setting_data.page_margin.top = FX_atof(argv_value);
    else if (argv_key.Equal("-mb")) pdf_setting_data.page_margin.bottom = FX_atof(argv_value);
    else if (argv_key.Equal("-scale")) pdf_setting_data.scaling_mode = (foxit::addon::conversion::HTML2PDFSettingData::HTML2PDFScalingMode)FXSYS_atoi(argv_value);
    else if (argv_key.Equal("-link")) pdf_setting_data.is_convert_link = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-tag")) pdf_setting_data.is_generate_tag = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-mode")) pdf_setting_data.page_mode = (foxit::addon::conversion::HTML2PDFSettingData::HTML2PDFPageMode)FXSYS_atoi(argv_value);
    else if (argv_key.Equal("-bookmarks")) pdf_setting_data.to_generate_bookmarks = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-encoding")) pdf_setting_data.encoding_format = (foxit::addon::conversion::HTML2PDFSettingData::HTML2PDFEncodingFormat)FXSYS_atoi(argv_value);
    else if (argv_key.Equal("-render_images")) pdf_setting_data.to_render_images = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-remove_underline_for_link")) pdf_setting_data.to_remove_underline_for_link = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-headerfooter")) pdf_setting_data.to_set_headerfooter = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-headerfooter_title")) pdf_setting_data.headerfooter_title = CFX_WideString::FromUTF8(argv_value);
    else if (argv_key.Equal("-headerfooter_url")) pdf_setting_data.headerfooter_url = CFX_WideString::FromUTF8(argv_value);
    else if (argv_key.Equal("-bookmark_root_name")) pdf_setting_data.bookmark_root_name = CFX_WideString::FromUTF8(argv_value);
    else if (argv_key.Equal("-resize_objects")) pdf_setting_data.to_resize_objects = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-print_background")) pdf_setting_data.to_print_background = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-optimize_tag")) pdf_setting_data.to_optimize_tag_tree = argv_value.EqualNoCase("yes") ? true : false;
    else if (argv_key.Equal("-media")) pdf_setting_data.media_style = (foxit::addon::conversion::HTML2PDFSettingData::HTML2PDFMediaStyle)FXSYS_atoi(argv_value);
    else if (argv_key.Equal("-engine")) engine_path = CFX_WideString::FromUTF8(argv_value);
    else if (argv_key.Equal("-cookies")) cookies_path = CFX_WideString::FromUTF8(argv_value);
    else if (argv_key.Equal("-o")) output_path = CFX_WideString::FromUTF8(argv_value);
    else if (argv_key.Equal("-html")) url_or_html = CFX_WideString::FromUTF8(argv_value);
    else if (argv_key.Equal("-timeout")) timeout = FXSYS_atoi(argv_value);
    else {
      HTML2PDFMSG
      return false;
    }
  }

  return true;
}

class FileWriter : public WriterCallback {
 public:
  FileWriter()
      : m_fileFP(NULL)
      , m_bLargeFile(FALSE) {}

  ~FileWriter() {}

  bool LoadFile(const char* filePath, bool bLargeFile = FALSE) {
#if defined(_WIN32) || defined(_WIN64)
    fopen_s(&m_fileFP, filePath, "wb");
#else
    m_fileFP = fopen(filePath, "wb");
#endif
    if (!m_fileFP)
      return FALSE;
    m_bLargeFile = bLargeFile;
    return TRUE;
  }

  FILESIZE GetSize() {
    if (!m_fileFP)
      return 0;
    if (m_bLargeFile) {
#if defined(_WIN32) || defined(_WIN64)
      _fseeki64(m_fileFP, 0, SEEK_END);
      long long sizeL = _ftelli64(m_fileFP);
#elif defined(__linux__) || defined(__APPLE__)
      fseeko(m_fileFP, 0, SEEK_END);
      long long sizeL = ftello(m_fileFP);
#endif
      return sizeL;
    } else {
      fseek(m_fileFP, 0, SEEK_END);
      return (uint32)ftell(m_fileFP);
    }
  }

  FX_BOOL Flush() {
    return fflush(m_fileFP) == 0;
  }

  FX_BOOL WriteBlock(const void* buffer, FILESIZE offset, size_t size) {
    if (!m_fileFP)
      return FALSE;
    if (m_bLargeFile) {
#if defined(_WIN32) || defined(_WIN64)
      _fseeki64(m_fileFP, offset, SEEK_SET);
#elif defined(__linux__) || defined(__APPLE__)
      fseeko(m_fileFP, offset, SEEK_SET);
#endif
      size_t readSize = fwrite(buffer, sizeof(char), size, m_fileFP);
      return (readSize == size);
    } else {
      if (!m_fileFP)
        return false;
      int ret1 = fseek(m_fileFP, (long)offset, SEEK_SET);
      uint64 write_size = fwrite(buffer, sizeof(char), size, m_fileFP);
      if (write_size == size) {
        return TRUE;
      }
    }
    return FALSE;
  }

  void Release() {
    if (m_fileFP)
      fclose(m_fileFP);
    m_fileFP = NULL;
    delete this;
  }

 private:
  FILE* m_fileFP;
  bool m_bLargeFile;
};

class FileReader : public ReaderCallback {
 public:
  FileReader()
      : m_fileFP(NULL)
      , m_bLargeFile(FALSE) {}

  ~FileReader() {}

  bool LoadFile(const wchar_t* wFilePath, bool bLargeFile = FALSE) {
    std::wstring strTemp(wFilePath);
    CFX_ByteString bytestring = CFX_ByteString::FromUnicode(wFilePath);
    string bstrFilepath = string(bytestring);
#if defined(_WIN32) || defined(_WIN64)
    fopen_s(&m_fileFP, bstrFilepath.c_str(), "rb");
#else
    m_fileFP = fopen(bstrFilepath.c_str(), "rb");
#endif
    if (!m_fileFP)
      return FALSE;
    m_bLargeFile = bLargeFile;
    return TRUE;
  }

  FILESIZE GetSize() {
    if (m_bLargeFile) {
#if defined(_WIN32) || defined(_WIN64)
      _fseeki64(m_fileFP, 0, SEEK_END);
      long long sizeL = _ftelli64(m_fileFP);
#elif defined(__linux__) || defined(__APPLE__)
      fseeko(m_fileFP, 0, SEEK_END);
      long long sizeL = ftello(m_fileFP);
#endif
      return sizeL;
    } else {
      fseek(m_fileFP, 0, SEEK_END);
      return (uint32)ftell(m_fileFP);
    }
  }

  FX_BOOL ReadBlock(void* buffer, FILESIZE offset, size_t size) {
    if (m_bLargeFile) {
#if defined(_WIN32) || defined(_WIN64)
      _fseeki64(m_fileFP, offset, SEEK_SET);
#elif defined(__linux__) || defined(__APPLE__)
      fseeko(m_fileFP, offset, SEEK_SET);
#endif
      long long readSize = fread(buffer, 1, size, m_fileFP);
      return (readSize == size);
    } else {
      if (!m_fileFP)
        return false;
      if (0 != fseek(m_fileFP, (long)offset, 0))
        return false;
      if (0 == fread(buffer, 1, size, m_fileFP))
        return false;
      return true;
    }
  }

  void Release() {
    if (m_fileFP)
      fclose(m_fileFP);
    m_fileFP = NULL;
    delete this;
  }

 private:
  FILE* m_fileFP;
  bool m_bLargeFile;
};

int main(int argc, char *argv[]) {
  if (argc > 1 && String(argv[1]).Equal("--help")) {
      cout << "Usage:" << endl << "html2pdf_xxx <-html <The url or html path>> <-o <output pdf path>> <-engine <htmltopdf engine path>>" << endl <<
          " [-w <page width>] [-h <page height>] [-ml <margin left>] [-mr <margin right>] [-mt <margin top>] [-mb <margin bottom>] [-r <page rotate degree>] [-mode <page mode>] [-scale <whether scale page>]" << endl <<
          " [-link <whether convert link>] [-tag <whether generate tag>] [-bookmarks <whether to generate bookmarks>] [-cookies <cookies file path>] [-timeout <timeout>]" << endl << endl <<
          "-html The url or html file path. for examples '-html www.foxitsoftware.com'" << endl <<
          "-o The output pdf path." << endl <<
          "-engine The html to pdf engine path." << endl <<
          "-w The page width." << endl <<
          "-h The page height." << endl <<
          "-r The page roate degree. '0' means 0 degree, '1' means 90 degree, '2' means 180 degree, '3' means 270 degree." << endl <<
          "-ml The page margin distance of left." << endl <<
          "-mr The page margin distance of right." << endl <<
          "-mt The page margin distance of top." << endl <<
          "-mb The page margin distance of bottom." << endl <<
          "-mode The page mode. 0 means single page mode and 1 means multiple mode" << endl <<
          "-scale The scaling mode. '0' means no scale, '1' means scale, '2' means enlarge page." << endl <<
          "-link Whether to covert link. 'yes' means to convert link, 'no' means no need to covert." << endl <<
          "-tag Whether to generate tag. 'yes' means to generate tag, 'no' means no need to generate." << endl <<
          "-bookmarks  Whether to generate bookmarks.'yes' means to generate bookmarks, 'no' means no need to generate." << endl <<
          "-encoding The HTML encoding format. '0' means auto encoding and '1'-'73' means other encodings" << endl <<
          "-render_images Whether to render images or not.'yes' means to render images, 'no' means no need to render." << endl <<
          "-remove_underline_for_link Whether to remove underline for link. 'yes' means to remove underline for link, 'no' means no need to remove." << endl <<
          "-headerfooter Whether to generate headerfooter. 'yes' means to generate headerfooter, 'no' means no need to generate." << endl <<
          "-headerfooter_title The headerfooter title." << endl <<
          "-headerfooter_url The headerfooter url." << endl <<
          "-bookmark_root_name The bookmark root name." << endl <<
          "-print_background Whether to print background. 'yes' means to print background, 'no' means no need to print background." << endl <<
          "-optimize_tag Whether to optimize tag tree. 'yes' means to optimize tag tree, 'no' means no need to optimize tag tree." << endl <<
          "-media The media style. '0' means screen media style, '1' means print media style." <<endl <<
          "-cookies The cookies file path." << endl <<
          "-timeout The timeout of loading webpages." << endl;
    return 0;
  }

  int err_ret = 0;
  //If using a stream to load the html file, set isfilestreamload to true
  bool isfilestreamload = false;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }
  WString output_directory = output_path + L"html2pdf/";
#if defined(_WIN32) || defined(_WIN64)
  _mkdir(String::FromUnicode(output_directory));
#else
  mkdir(String::FromUnicode(output_directory), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif

  WString url_or_html = input_path + L"AboutFoxit.html";

  // "engine_path" is the path of the engine file "fxhtml2pdf" which is used to convert html to pdf. Please refer to Developer Guide for more details.
  WString engine_path = L"";

  // "cookies_path" is the path of the cookies file exported from the web pages that you want to convert. Please refer to Developer Guide for more details.
  WString cookies_path = L"";

  WString output_path = output_directory + L"html2pdf_result.pdf";

  int time_out = 15;

  try {
    foxit::addon::conversion::HTML2PDFSettingData pdf_setting_data;
    pdf_setting_data.is_convert_link = true;
    pdf_setting_data.is_generate_tag = true;
    pdf_setting_data.to_generate_bookmarks = true;
    pdf_setting_data.rotate_degrees = foxit::common::e_Rotation0;
    pdf_setting_data.page_height = 640;
    pdf_setting_data.page_width = 900;
    pdf_setting_data.page_mode = foxit::addon::conversion::HTML2PDFSettingData::e_PageModeSinglePage;
    pdf_setting_data.scaling_mode = foxit::addon::conversion::HTML2PDFSettingData::e_ScalingModeScale;
    pdf_setting_data.to_print_background = true;
    pdf_setting_data.to_optimize_tag_tree = false;
    pdf_setting_data.media_style = foxit::addon::conversion::HTML2PDFSettingData::e_MediaStyleScreen;

    if (!AnalysisParameter(argc, argv, pdf_setting_data, url_or_html, engine_path, cookies_path, output_path, time_out)) return 1;
    foxit::addon::conversion::Convert::FromHTML(url_or_html, engine_path, cookies_path, pdf_setting_data, output_path, time_out);

    cout << "Convert HTML to PDF successfully." << endl;
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  } catch (...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }
  //get HTML data from stream
  if (isfilestreamload) {
    try {
      foxit::addon::conversion::HTML2PDFSettingData pdf_setting_data;
      pdf_setting_data.page_height = 650;
      pdf_setting_data.page_width = 950;
      pdf_setting_data.is_to_page_scale = false;
      pdf_setting_data.page_margin = RectF(18, 18, 18, 18);
      pdf_setting_data.is_convert_link = true;
      pdf_setting_data.rotate_degrees = foxit::common::e_Rotation0;
      pdf_setting_data.is_generate_tag = true;

      foxit::addon::conversion::HTML2PDFSettingData::HTML2PDFPageMode mode =
          foxit::addon::conversion::HTML2PDFSettingData::e_PageModeSinglePage;
      pdf_setting_data.to_generate_bookmarks = true;
      foxit::addon::conversion::HTML2PDFSettingData::HTML2PDFScalingMode scale =
          foxit::addon::conversion::HTML2PDFSettingData::e_ScalingModeNone;
      pdf_setting_data.encoding_format = foxit::addon::conversion::HTML2PDFSettingData::e_EncodingFormatDefault;
      pdf_setting_data.to_render_images = true;
      pdf_setting_data.to_remove_underline_for_link = false;
      pdf_setting_data.to_set_headerfooter = false;
      pdf_setting_data.headerfooter_title = L"";
      pdf_setting_data.headerfooter_url = L"";
      pdf_setting_data.bookmark_root_name = L"abcde";
      pdf_setting_data.to_resize_objects = true;
      pdf_setting_data.to_print_background = false;
      pdf_setting_data.to_optimize_tag_tree = false;
      pdf_setting_data.media_style = foxit::addon::conversion::HTML2PDFSettingData::e_MediaStyleScreen;
      pdf_setting_data.to_load_active_content = false;
      WString cookies = L"";
      WString output_path = output_directory + L"html2pdf_filestream_result.pdf";
      FileWriter* filewrite = new FileWriter();
      filewrite->LoadFile(String::FromUnicode(output_path));
      // "htmlfile" is the path of the html file to be loaded. For example: "C:/aaa.html". The method of "FromHTML" will load this file as a stream.
      WString htmlfile = L"";
      FileReader* filereader = new FileReader();
      filereader->LoadFile(htmlfile);

      foxit::addon::conversion::HTML2PDFRelatedResourceArray html2PDFRelatedResourceArray;
      foxit::addon::conversion::HTML2PDFRelatedResource html2PDFRelatedResource;

      //"htmlfilepng" is the path of the png resource file to be loaded. For example: "C:/aaa.png". set "htmlfilepng" in the related_resource_file of HTML2PDFRelatedResource.
      WString htmlfilepng = L"";
      FileReader* filereader1 = new FileReader();
      filereader1->LoadFile(htmlfilepng);
      html2PDFRelatedResource.related_resource_file = filereader1;
      //"relativefilepath" is  the resource file's relative path. For example: "./aaa.png".
      WString relativefilepath = L"";
      html2PDFRelatedResource.resource_file_relative_path = relativefilepath;
      html2PDFRelatedResourceArray.Add(html2PDFRelatedResource);

      foxit::addon::conversion::Convert::FromHTML(filereader, html2PDFRelatedResourceArray, engine_path, NULL,
                                                  pdf_setting_data, filewrite, 30);
      cout << "Convert HTML to PDF successfully by filestream." << endl;
    } catch (const Exception& e) {
      cout << e.GetMessage() << endl;
      err_ret = 1;
    } catch (...) {
      cout << "Unknown Exception" << endl;
      err_ret = 1;
    }
  }

  return err_ret;
}
#endif

