// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to save a PDF document as a wrapper file
// and then open the wrapper file.

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
#include "../../../include/common/fs_render.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace common::file;

static const char* sn = "";
static const char* key = "";

struct PdfWrapperCommand {
  WString input_file;
  WString output_file;
  WString action;          // wrap, payload, info, extract
  // wrap options
  WString wrapper_type;    // WrapperData type (default: Foxit)
  WString wrapper_app_id;  // WrapperData app_id (default: Foxit)
  WString wrapper_uri;     // WrapperData uri (default: www.foxitsoftware.com)
  WString wrapper_desc;    // WrapperData description (default: foxit)
  int wrapper_version;     // WrapperData version (default: 10)
  uint32 permissions;      // user permissions (default: 0xFFFFFFFC)
  // payload options
  WString payload_file;    // payload PDF file path
  WString crypto_filter;   // crypto filter name (default: Unknown)
  float payload_version;   // payload version (default: 1.0)
  bool show_help;

  PdfWrapperCommand()
      : wrapper_version(10),
        permissions(0xFFFFFFFC),
        payload_version(1.0f),
        show_help(false) {}
};

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

void PrintUsage() {
  cout << "Usage:" << endl;
  cout << "pdfwrapper --action <wrap|payload|info|extract> --input <input.pdf> --output <output.pdf> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --action <type>                 Action: wrap | payload | info | extract." << endl;
  cout << "  --input <path>                  Input PDF file path." << endl;
  cout << "  --output <path>                 Output file path (for wrap action, must be an existing PDF file)." << endl << endl;
  cout << "Wrap options:" << endl;
  cout << "  --wrapper-type <text>           Wrapper type. Default: Foxit." << endl;
  cout << "  --wrapper-app-id <text>         Application ID. Default: Foxit." << endl;
  cout << "  --wrapper-uri <text>            URI. Default: www.foxitsoftware.com." << endl;
  cout << "  --wrapper-desc <text>           Description. Default: foxit." << endl;
  cout << "  --wrapper-version <int>         Wrapper version. Default: 10." << endl;
  cout << "  --permissions <hex>             User permissions. Default: 0xFFFFFFFC." << endl << endl;
  cout << "Payload options:" << endl;
  cout << "  --payload-file <path>           Payload PDF file path (required for payload action)." << endl;
  cout << "  --crypto-filter <text>          Crypto filter name. Default: Unknown." << endl;
  cout << "  --payload-version <float>       Payload version. Default: 1.0." << endl << endl;
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

uint32 ParseHexUint32(const String& value) {
  if (value.GetLength() > 2 && (value.GetAt(0) == '0' && (value.GetAt(1) == 'x' || value.GetAt(1) == 'X'))) {
    return (uint32)strtoul((const char*)value, NULL, 0);
  }
  return (uint32)atoi((const char*)value);
}

bool ParseCommand(int argc, char* argv[], PdfWrapperCommand& command) {
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
      if (!value.Equal("wrap") && !value.Equal("payload") && !value.Equal("info") && !value.Equal("extract")) {
        printf("Invalid action: %s (must be wrap, payload, info, or extract)\n", (const char*)value);
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
    } else if (key.Equal("--wrapper-type")) {
      command.wrapper_type = WString::FromUTF8(value);
    } else if (key.Equal("--wrapper-app-id")) {
      command.wrapper_app_id = WString::FromUTF8(value);
    } else if (key.Equal("--wrapper-uri")) {
      command.wrapper_uri = WString::FromUTF8(value);
    } else if (key.Equal("--wrapper-desc")) {
      command.wrapper_desc = WString::FromUTF8(value);
    } else if (key.Equal("--wrapper-version")) {
      command.wrapper_version = atoi((const char*)value);
    } else if (key.Equal("--permissions")) {
      command.permissions = ParseHexUint32(value);
    } else if (key.Equal("--payload-file")) {
      command.payload_file = WString::FromUTF8(value);
    } else if (key.Equal("--crypto-filter")) {
      command.crypto_filter = WString::FromUTF8(value);
    } else if (key.Equal("--payload-version")) {
      command.payload_version = (float)atof((const char*)value);
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
  if (command.action.Equal(L"wrap") && !FileExists(command.output_file)) {
    printf("For wrap action, output file must be an existing PDF file: %s\n",
           (const char*)String::FromUnicode(command.output_file));
    return false;
  }
  return true;
}

void RenderPDF2Img(PDFDoc doc, const WString& out_dir) {
  int page_count = doc.GetPageCount();

  for (int i = 0; i < page_count; i++) {
    PDFPage page = doc.GetPage(i);

    // Parse page.
    page.StartParse(foxit::pdf::PDFPage::e_ParsePageNormal, NULL, false);

    int width = static_cast<int>(page.GetWidth());
    int height = static_cast<int>(page.GetHeight());
    Matrix matrix = page.GetDisplayMatrix(0, 0, width, height, page.GetRotation());

    // Prepare a bitmap for rendering.
    Bitmap bitmap(width, height, foxit::common::Bitmap::e_DIBArgb, NULL, 0);
    bitmap.FillRect(0xFFFFFFFF, NULL);

    // Render page.
    Renderer render(bitmap, false);
    render.StartRender(page, matrix, NULL);
    // Add the bitmap to an image and save the image.
    Image image;
    image.AddFrame(bitmap);
    WString image_name;
    image_name.Format((FX_LPCWSTR)L"page_%d",i);
    image_name = out_dir + image_name + L"_wrapper.jpg";

    if (!image.SaveAs(image_name)) {
      printf("Failed to save image for page %d.\n", i);
    }
  }

}

class FileReader : public ReaderCallback
{
public:
  FileReader(int64 offset) : file_(NULL)
                           , filesize_(offset) {}
  ~FileReader() {}

  bool LoadFile(const char* file_path) {
#if defined(_WIN32) || defined(_WIN64)
    fopen_s(&file_, file_path, "rb");
#else
    file_ = fopen(file_path, "rb");
#endif
    if (!file_) return FALSE;
    return TRUE;
  }

  FILESIZE  GetSize()
  {
    return filesize_;
  }

  FX_BOOL ReadBlock(void* buffer, FILESIZE offset, size_t size)
  {
    if (!file_) return 0;
    if(0 != fseek(file_, (long)offset, 0))
      return 0;
    if(0 == fread(buffer, size, 1, file_))
      return 0;
    return 1;
  }

  size_t ReadBlock(void* buffer, size_t size) {
    if (!file_) return false;
    if(0 != fseek(file_, 0, 0))
      return 0;
    return fread(buffer, size, 1, file_);
  }

  void Release() {
    if(file_)
      fclose(file_);
    file_ = NULL;
  }

private:
  FILE* file_;
  int64 filesize_;
};

class FileWriter : public WriterCallback
{
public:
  FileWriter(): file_(NULL)
  {}

  ~FileWriter() {}

  bool LoadFile(const char* file_path) {
#if defined(_WIN32) || defined(_WIN64)
    fopen_s(&file_, file_path, "wb");
#else
    file_ = fopen(file_path, "wb");
#endif
    if (!file_) return FALSE;
    return TRUE;
  }

  FILESIZE GetSize() {
    if(!file_) return 0;
    fseek(file_, 0, SEEK_END);
    return (uint32)ftell(file_);
  }

  FX_BOOL Flush() {
    return TRUE;
  }

  FX_BOOL WriteBlock(const void* buffer, FILESIZE offset, size_t size) {
    if(!file_) return FALSE;

    fseek(file_, (long)offset, SEEK_SET);
    uint64 write_size = fwrite(buffer, sizeof(char), size, file_);
    if(write_size == size) {
      return TRUE;
    }

    return FALSE;
  }

  FX_BOOL WriteBlock(const void* pData, size_t size) {
    return WriteBlock(pData, GetSize(), size);
  }

  void Release() {
    if(file_) fclose(file_);
    file_ = NULL;
    delete this;
  }

private:
  FILE* file_;
};

bool OpenWrapperFile(WString file_name, const WString& out_dir) {
  PDFDoc doc(file_name);
  ErrorCode code = doc.Load();
  if (code != foxit::e_ErrSuccess) {
    printf("Failed to load PDF document: %s (Error: %d)\n", (const char*)String::FromUnicode(file_name), code);
    return false;
  }
  if (!doc.IsWrapper()) {
    printf("The document is not a wrapper file: %s\n", (const char*)String::FromUnicode(file_name));
    return false;
  }
  int64 offset = doc.GetWrapperOffset();
  WrapperData wrapper_data = doc.GetWrapperData();

  printf("Wrapper info:\n");
  printf("  Offset: %lld\n", (long long)offset);
  printf("  Version: %d\n", wrapper_data.version);
  printf("  Type: %s\n", (const char*)String::FromUnicode(wrapper_data.type));
  printf("  App ID: %s\n", (const char*)String::FromUnicode(wrapper_data.app_id));
  printf("  URI: %s\n", (const char*)String::FromUnicode(wrapper_data.uri));
  printf("  Description: %s\n", (const char*)String::FromUnicode(wrapper_data.description));

  //"offset" can also indicate the end position of the document or the document size
  FileReader file_reader(offset);
  file_reader.LoadFile(String::FromUnicode(file_name));

  PDFDoc doc_real(&file_reader);
  code = doc_real.Load();
  if (code != foxit::e_ErrSuccess) {
    printf("Failed to load real PDF document (Error: %d)\n", code);
    return false;
  }

  RenderPDF2Img(doc_real, out_dir);
  return true;
}

int main(int argc, char *argv[])
{
  PdfWrapperCommand command;
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
    if (command.action.Equal(L"wrap")) {
      // Load the input PDF document.
      PDFDoc doc(command.input_file);
      error_code = doc.Load();
      if (error_code != foxit::e_ErrSuccess) {
        printf("Failed to load PDF document: %s (Error: %d)\n",
               (const char*)String::FromUnicode(command.input_file), error_code);
        return 1;
      }

      // Build WrapperData from parameters (use defaults if not specified).
      WString wtype = command.wrapper_type.IsEmpty() ? L"Foxit" : command.wrapper_type;
      WString wappid = command.wrapper_app_id.IsEmpty() ? L"Foxit" : command.wrapper_app_id;
      WString wuri = command.wrapper_uri.IsEmpty() ? L"www.foxitsoftware.com" : command.wrapper_uri;
      WString wdesc = command.wrapper_desc.IsEmpty() ? L"foxit" : command.wrapper_desc;
      WrapperData wrapper_data(command.wrapper_version, wtype, wappid, wuri, wdesc);

      // Save as wrapper file.
      doc.SaveAsWrapperFile(command.output_file, &wrapper_data, command.permissions);
      printf("Saved wrapper file: %s\n", (const char*)String::FromUnicode(command.output_file));

      // Open and verify the wrapper file.
      if (!OpenWrapperFile(command.output_file, output_dir)) {
        return 1;
      }
      printf("Wrap action completed.\n");

    } else if (command.action.Equal(L"payload")) {
      // Load the input PDF document.
      PDFDoc doc(command.input_file);
      error_code = doc.Load();
      if (error_code != foxit::e_ErrSuccess) {
        printf("Failed to load PDF document: %s (Error: %d)\n",
               (const char*)String::FromUnicode(command.input_file), error_code);
        return 1;
      }

      if (command.payload_file.IsEmpty()) {
        printf("--payload-file is required for payload action.\n");
        return 1;
      }
      if (!FileExists(command.payload_file)) {
        printf("Payload file does not exist: %s\n", (const char*)String::FromUnicode(command.payload_file));
        return 1;
      }

      WString cfilter = command.crypto_filter.IsEmpty() ? L"Unknown" : command.crypto_filter;
      doc.StartSaveAsPayloadFile(command.output_file, command.payload_file,
                                  cfilter, L"no_description", command.payload_version,
                                  foxit::pdf::PDFDoc::e_SaveFlagIncremental, NULL);
      printf("Saved payload file: %s\n", (const char*)String::FromUnicode(command.output_file));
      printf("Payload action completed.\n");

    } else if (command.action.Equal(L"info")) {
      // Open the wrapper file and display info.
      if (!OpenWrapperFile(command.input_file, output_dir)) {
        return 1;
      }
      printf("Info action completed.\n");

    } else if (command.action.Equal(L"extract")) {
      // Load the wrapper file and extract payload.
      PDFDoc doc(command.input_file);
      error_code = doc.Load();
      if (error_code != foxit::e_ErrSuccess) {
        printf("Failed to load PDF document: %s (Error: %d)\n",
               (const char*)String::FromUnicode(command.input_file), error_code);
        return 1;
      }

      if (foxit::pdf::PDFDoc::e_WrapperPDFV2 == doc.GetWrapperType()) {
        FileWriter* payloadfile = new FileWriter();
        payloadfile->LoadFile(String::FromUnicode(command.output_file));
        doc.StartGetPayloadFile(payloadfile, NULL);
        printf("Extracted payload file: %s\n", (const char*)String::FromUnicode(command.output_file));
      } else {
        printf("The input file is not an RMS V2 wrapper. Cannot extract payload.\n");
        return 1;
      }
      printf("Extract action completed.\n");
    }

  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  }
  catch (...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}

