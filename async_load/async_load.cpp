// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to loading PDF document asynchronously.

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
#include "../../../include/common/fs_render.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

// CLI parameter globals
static std::string g_input_file;
static std::string g_output_path;

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

static bool ParseArgs(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    foxit::String arg(argv[i]);
    if (arg.Equal("--help")) {
      printf(
        "Usage: async_load_xxx [options]\n"
        "\n"
        "Required options:\n"
        "  -i <file>   Input PDF file path\n"
        "  -o <file>   Output rendered page image path (e.g. page0.bmp)\n"
        "\n"
        "Optional options:\n"
        "  --help      Show this help and exit\n"
        "\n"
        "Examples:\n"
        "  -i input.pdf -o output/page0.bmp\n"
      );
      exit(0);
    }

#define NEED_VAL(opt) \
    do { if (i + 1 >= argc) { \
      printf("Missing value for '%s'.\nTry 'async_load_xxx --help' for more information.\n", (opt)); \
      return false; \
    } } while(0)

    if (arg.Equal("-i")) {
      NEED_VAL("-i"); ++i;
      g_input_file = argv[i];
    } else if (arg.Equal("-o")) {
      NEED_VAL("-o"); ++i;
      g_output_path = argv[i];
    } else {
      printf("Unknown argument: '%s'.\nTry 'async_load_xxx --help' for more information.\n", argv[i]);
      return false;
    }
  }
  return true;
}

// Data of asynchronous loader callback object.
struct DownloadHintDataInfo {
  DownloadHintDataInfo() {
    offset = 0;
    size = 0;
    is_downloaded = false;
  }
  DownloadHintDataInfo(int64 offset, int64 size, bool is_downloaded) {
    this->offset = offset;
    this->size = size;
    this->is_downloaded = is_downloaded;
  }
  int64 offset;
  int64 size;
  bool is_downloaded;
};

// This callback just simulates the downloading progress by DownloadHintDataInfo::is_downloaded,
// not really to download data.
class AsyncFileRead : public foxit::common::file::AsyncReaderCallback {
public:
  AsyncFileRead()
    : file_(NULL)
    , is_large_file_(false) {}

  ~AsyncFileRead() {
    hint_data_record_.clear();
  }

  bool LoadFile(const wchar_t* file_path, bool is_large_file = false) {
#if defined(_WIN32) || (_WIN64)
    fopen_s(&file_, String::FromUnicode(file_path), "rb");
#else
    file_ = fopen(String::FromUnicode(file_path), "rb");
#endif
    if (!file_)
      return false;

    is_large_file_ = is_large_file;
    return true;
  }

  FILESIZE GetSize() {
    if (is_large_file_) {
#if defined(_WIN32) || defined(_WIN64)
      _fseeki64(file_, 0, SEEK_END);
      long long size_long = _ftelli64(file_);
#elif defined(__linux__) || defined(__APPLE__)
      fseeko(file_, 0, SEEK_END);
      long long size_long = ftello(file_);
#endif
      return size_long;
    } else {
      fseek(file_, 0, SEEK_END);
      return (int64)ftell(file_);
    }
    return 0;
  }

  FX_BOOL      ReadBlock(void* buffer, FILESIZE offset, size_t size) {
    if (is_large_file_) {
#if defined(_WIN32) || defined(_WIN64)
      _fseeki64(file_, offset, SEEK_SET);
#elif defined(__linux__) || defined(__APPLE__)
      fseeko(file_, offset, SEEK_SET);
#endif
      long long read_size = fread(buffer, 1, size, file_);
      return read_size == size ? true : false;
    } else {
      if (!file_)
        return false;
      if (0 != fseek(file_, static_cast<long>(offset), 0))
        return false;
      if (0 == fread(buffer, size, 1, file_))
        return false;
      return true;
    }
    return false;
  }

  void Release() {
    if (file_)
      fclose(file_);
    file_ = NULL;
    delete this;
  }

  virtual bool IsDataAvail(int64 offset, size_t size) {
    return CheckRecordDownloaded(offset, size, false);
  }
  virtual bool AddDownloadHint(int64 offset, size_t size) {
    // Record the range and downloaded data.
    return CheckRecordDownloaded(offset, size, true);
  }

protected:
  bool CheckRecordDownloaded(int64 offset, int64 size, bool to_download) {
    size_t record_count = hint_data_record_.size();
    for (size_t i = 0; i < record_count; i++) {
      DownloadHintDataInfo data_info = hint_data_record_[i];

      // If (offset+size) is out of current data_info, just continue to check other record in hint_data_record_.
      if (offset > (data_info.offset + data_info.size))
        continue;
      if (offset + size < data_info.offset)
        continue;

      // If data defined by <offset, size> has been in/within current data info, just download the data.
      if (offset >= data_info.offset && (offset + size) <= (data_info.offset + data_info.size)) {
        if (to_download)
          data_info.is_downloaded = true;
        return data_info.is_downloaded;
      }

      // If only part of data defined by <offset, size> is in current data_info, download current data_info and
      // also check and download rest data.
      if (offset >= data_info.offset && offset < (data_info.offset + data_info.size) &&
        (offset + size) > (data_info.offset + data_info.size)) {
          if (to_download)
            data_info.is_downloaded = true;
          if (!data_info.is_downloaded)
            return data_info.is_downloaded;
          int64 new_offset = data_info.offset + data_info.size + 1;
          int64 new_size = size - 1 - (data_info.offset + data_info.size - offset);
          return CheckRecordDownloaded(new_offset, new_size, to_download);
      }

      if (offset < data_info.offset && (offset + size) >= data_info.offset &&
        (offset + size) <= (data_info.offset + data_info.size)) {
          if (to_download)
            data_info.is_downloaded = true;
          if (!data_info.is_downloaded)
            return data_info.is_downloaded;
          int64 new_offset = offset;
          int64 new_size = data_info.offset - 1 - offset;
          return CheckRecordDownloaded(new_offset, new_size, to_download);
      }
      if (offset < data_info.offset && (offset + size) > (data_info.offset + data_info.size)) {
        if (to_download)
          data_info.is_downloaded = true;
        if (!data_info.is_downloaded)
          return data_info.is_downloaded;
        int64 new_offset = offset;
        int64 new_size = data_info.offset - 1 - offset;
        if (CheckRecordDownloaded(new_offset, new_size, to_download)) {
          new_offset = data_info.offset + data_info.size + 1;
          new_size = size - 1 - (data_info.offset - offset + data_info.size);
          return CheckRecordDownloaded(new_offset, new_size, to_download);
        } else
          return false;
      }
    }
    if (to_download) {
      return DownloadData(offset, size);
    }
    return false;
  }
  bool DownloadData(int64 offset, int64 size) {
    DownloadHintDataInfo new_info(offset, size, true);
    hint_data_record_.push_back(new_info);
    return true;
  }

private:
  // Used to record added hint range and if the range is downloaded.
  std::vector<DownloadHintDataInfo> hint_data_record_;
  FILE* file_;
  bool is_large_file_;
};

int main(int argc, char* argv[]) {
  if (!ParseArgs(argc, argv)) return 1;

  if (g_input_file.empty()) {
    printf("Error: -i (input file) is required.\nTry 'async_load_xxx --help' for more information.\n");
    return 1;
  }
  if (g_output_path.empty()) {
    printf("Error: -o (output image path) is required.\nTry 'async_load_xxx --help' for more information.\n");
    return 1;
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

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

  WString input_file = WString::FromLocal(g_input_file.c_str());
  WString output_file = WString::FromLocal(g_output_path.c_str());

  try {
    AsyncFileRead* file_read = new AsyncFileRead();
    file_read->LoadFile(input_file);
    PDFDoc doc(file_read, true);

    // Actually, here, application should download needed data which specified by AsyncFileRead::AddDownloadHint().
    // But here, for simple example, we just "download" these data inside AsyncFileRead::AddDownloadHint().
    // So, just continue to check the ready state here, which will trigger AsyncFileRead::AddDownloadHint() to
    // "download" data.
    ErrorCode code = foxit::e_ErrDataNotReady;
    while (code == foxit::e_ErrDataNotReady)
      code = doc.Load();
    if (code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", g_input_file.c_str(), error_code);
      return 1;
    }

    // Actually, here, application should download needed data which specified by AsyncFileRead::AddDownloadHint().
    // But here, for simple example, we just "download" these data inside AsyncFileRead::AddDownloadHint().
    // So, just continue to check the ready state here, which will trigger AsyncFileRead::AddDownloadHint() to
    // "download" data.
    PDFPage page;
    code = foxit::e_ErrDataNotReady;
    while (code == foxit::e_ErrDataNotReady) {
      try {
        page = doc.GetPage(0);
        break;
      }
      catch (Exception e) {
        code = e.GetErrCode();
      }
      catch (...) {
        throw;
      }
    }
    // Parse page.
    page.StartParse(foxit::pdf::PDFPage::e_ParsePageNormal, NULL, false);

    int width = static_cast<int>(page.GetWidth());
    int height = static_cast<int>(page.GetHeight());
    foxit::Matrix matrix = page.GetDisplayMatrix(0, 0, width, height, page.GetRotation());

    // Prepare a bitmap for rendering.
    foxit::common::Bitmap bitmap(width, height, foxit::common::Bitmap::e_DIBArgb, NULL, 0);
    bitmap.FillRect(0xFFFFFFFF, NULL);
    // Render page
    foxit::common::Renderer render(bitmap, false);
    render.StartRender(page, matrix, NULL);

    foxit::common::Image image;
    image.AddFrame(bitmap);
    image.SaveAs(output_file);

    cout << "async-load demo finished." << endl;

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

