// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to do optimize in PDF documents under a folder.

// Include Foxit SDK header files.
#include <time.h>
#include <iostream>
#include <cstdlib>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/addon/optimization/fs_optimization.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace foxit::addon;
using namespace foxit::addon::optimization;

struct ImageSettingsArgs {
  int dpi;           // -1 = not set
  int dpi_limit;     // -1 = not set
  int compress_mode; // -1 = not set (ImageSettings::ImageCompressMode)
  int quality;       // -1 = not set (ImageSettings::ImageCompressQuality)
  ImageSettingsArgs() : dpi(-1), dpi_limit(-1), compress_mode(-1), quality(-1) {}
  bool HasAny() const { return dpi != -1 || dpi_limit != -1 || compress_mode != -1 || quality != -1; }
};

struct MonoImageSettingsArgs {
  int dpi;           // -1 = not set
  int dpi_limit;     // -1 = not set
  int compress_mode; // -1 = not set (MonoImageSettings::MonoImageCompressMode)
  int quality;       // -1 = not set (MonoImageSettings::MonoImageCompressQuality)
  MonoImageSettingsArgs() : dpi(-1), dpi_limit(-1), compress_mode(-1), quality(-1) {}
  bool HasAny() const { return dpi != -1 || dpi_limit != -1 || compress_mode != -1 || quality != -1; }
};

struct OptimizationCommand {
  WString input_file;
  WString output_file;
  int optimizer_options;
  int cleanup_options;
  int discard_objects_options;
  int discard_userdata_options;  // for SetDiscardUserDataOptions
  int transparency_mode;         // -1=not set, 0=low,1=medium,2=high
  ImageSettingsArgs color_img;
  ImageSettingsArgs gray_img;
  MonoImageSettingsArgs mono_img;
  int font_subset_all;           // -1=not set, 0=false, 1=true
  bool has_input;
  bool has_output;
  bool has_options;
  bool has_cleanup_options;
  bool has_discard_objects_options;
  bool has_discard_userdata_options;

  OptimizationCommand()
      : optimizer_options(0),
        cleanup_options(0),
        discard_objects_options(0),
        discard_userdata_options(0),
        transparency_mode(-1),
        font_subset_all(-1),
        has_input(false),
        has_output(false),
        has_options(false),
        has_cleanup_options(false),
        has_discard_objects_options(false),
        has_discard_userdata_options(false) {}
};

void PrintUsage() {
  cout << "Usage:" << endl
      << "optimization --input <input pdf path> --output <output pdf path> --options <optimizer option mask> [optional settings]" << endl
      << endl
      << "Required:" << endl
      << "--input <path>                     Input pdf file path." << endl
      << "--output <path>                    Output pdf file path." << endl
      << "--options <int>                    Optimizer options bitmask." << endl
      << endl
      << "Optional:" << endl
      << "--cleanup-options <int>            Cleanup options bitmask (for cleanup optimization)." << endl
      << "--discard-objects-options <int>    Discard object options bitmask (for discard optimization)." << endl
      << "--discard-userdata-options <int>   Discard user data options bitmask." << endl
      << "--transparency-mode <low|medium|high>  Transparency flattening mode." << endl
      << endl
      << "Color image options (--options must include 0x01):" << endl
      << "--color-img-dpi <int>              Target DPI for color images (default: 150)." << endl
      << "--color-img-dpi-limit <int>        Lower limit DPI to trigger compression for color images." << endl
      << "--color-img-compress <mode>        Compression mode: high|jpeg|jpeg2000|retain|zip (default: jpeg)." << endl
      << "--color-img-quality <q>            Quality: minimum|low|medium|high|maximum|lossless (default: minimum)." << endl
      << endl
      << "Grayscale image options (--options must include 0x01):" << endl
      << "--gray-img-dpi <int>               Target DPI for grayscale images." << endl
      << "--gray-img-dpi-limit <int>         Lower limit DPI to trigger compression for grayscale images." << endl
      << "--gray-img-compress <mode>         Compression mode: high|jpeg|jpeg2000|retain|zip." << endl
      << "--gray-img-quality <q>             Quality: minimum|low|medium|high|maximum|lossless." << endl
      << endl
      << "Monochrome image options (--options must include 0x01):" << endl
      << "--mono-img-dpi <int>               Target DPI for monochrome images." << endl
      << "--mono-img-dpi-limit <int>         Lower limit DPI to trigger compression for monochrome images." << endl
      << "--mono-img-compress <mode>         Compression mode: ccitt3|ccitt4|high|jbig2|retain|runlength|zip (default: ccitt4)." << endl
      << "--mono-img-quality <q>             Quality: lossless|lossy|minimum|low|medium|high|maximum (default: lossless)." << endl
      << endl
      << "Font options (--options must include 0x08):" << endl
      << "--font-subset-all <true|false>     Subset all unembedded fonts." << endl;
}

bool ParseIntValue(const String& value, int& out_value) {
  char* end_ptr = NULL;
  out_value = static_cast<int>(strtol((const char*)value, &end_ptr, 10));
  return end_ptr != NULL && *end_ptr == '\0';
}

bool ParseBoolValue(const String& value, bool& out_value) {
  if (value.Equal("true") || value.Equal("1")) { out_value = true; return true; }
  if (value.Equal("false") || value.Equal("0")) { out_value = false; return true; }
  return false;
}

bool ParseImgCompressMode(const String& value, int& out) {
  if (value.Equal("high"))     { out = 10003; return true; }
  if (value.Equal("jpeg"))     { out = 10005; return true; }
  if (value.Equal("jpeg2000")) { out = 10006; return true; }
  if (value.Equal("retain"))   { out = 10007; return true; }
  if (value.Equal("zip"))      { out = 10009; return true; }
  return false;
}

bool ParseImgQuality(const String& value, int& out) {
  if (value.Equal("minimum"))  { out = 1; return true; }
  if (value.Equal("low"))      { out = 2; return true; }
  if (value.Equal("medium"))   { out = 3; return true; }
  if (value.Equal("high"))     { out = 4; return true; }
  if (value.Equal("maximum"))  { out = 5; return true; }
  if (value.Equal("lossless")) { out = 6; return true; }
  return false;
}

bool ParseMonoCompressMode(const String& value, int& out) {
  if (value.Equal("ccitt3"))    { out = 10001; return true; }
  if (value.Equal("ccitt4"))    { out = 10002; return true; }
  if (value.Equal("high"))      { out = 10003; return true; }
  if (value.Equal("jbig2"))     { out = 10004; return true; }
  if (value.Equal("retain"))    { out = 10007; return true; }
  if (value.Equal("runlength")) { out = 10008; return true; }
  if (value.Equal("zip"))       { out = 10009; return true; }
  return false;
}

bool ParseMonoQuality(const String& value, int& out) {
  if (value.Equal("lossless")) { out = 1; return true; }
  if (value.Equal("lossy"))    { out = 2; return true; }
  if (value.Equal("minimum"))  { out = 3; return true; }
  if (value.Equal("low"))      { out = 4; return true; }
  if (value.Equal("medium"))   { out = 5; return true; }
  if (value.Equal("high"))     { out = 6; return true; }
  if (value.Equal("maximum"))  { out = 7; return true; }
  return false;
}

bool AnalysisParameter(int argc, char* argv[], OptimizationCommand& command) {
  if (argc < 7 || ((argc - 1) % 2 != 0)) {
    return false;
  }

  for (int i = 1; i < argc; i += 2) {
    String key = String(argv[i]);
    String value = String(argv[i + 1]);

    if (key.Equal("--input")) {
      command.input_file = WString::FromUTF8(value);
      command.has_input = true;
    } else if (key.Equal("--output")) {
      command.output_file = WString::FromUTF8(value);
      command.has_output = true;
    } else if (key.Equal("--options")) {
      if (!ParseIntValue(value, command.optimizer_options)) {
        return false;
      }
      command.has_options = true;
    } else if (key.Equal("--cleanup-options")) {
      if (!ParseIntValue(value, command.cleanup_options)) {
        return false;
      }
      command.has_cleanup_options = true;
    } else if (key.Equal("--discard-objects-options")) {
      if (!ParseIntValue(value, command.discard_objects_options)) {
        return false;
      }
      command.has_discard_objects_options = true;
    } else if (key.Equal("--discard-userdata-options")) {
      if (!ParseIntValue(value, command.discard_userdata_options)) return false;
      command.has_discard_userdata_options = true;
    } else if (key.Equal("--transparency-mode")) {
      if (value.Equal("low"))         command.transparency_mode = 0;
      else if (value.Equal("medium")) command.transparency_mode = 1;
      else if (value.Equal("high"))   command.transparency_mode = 2;
      else return false;
    } else if (key.Equal("--color-img-dpi")) {
      if (!ParseIntValue(value, command.color_img.dpi)) return false;
    } else if (key.Equal("--color-img-dpi-limit")) {
      if (!ParseIntValue(value, command.color_img.dpi_limit)) return false;
    } else if (key.Equal("--color-img-compress")) {
      if (!ParseImgCompressMode(value, command.color_img.compress_mode)) return false;
    } else if (key.Equal("--color-img-quality")) {
      if (!ParseImgQuality(value, command.color_img.quality)) return false;
    } else if (key.Equal("--gray-img-dpi")) {
      if (!ParseIntValue(value, command.gray_img.dpi)) return false;
    } else if (key.Equal("--gray-img-dpi-limit")) {
      if (!ParseIntValue(value, command.gray_img.dpi_limit)) return false;
    } else if (key.Equal("--gray-img-compress")) {
      if (!ParseImgCompressMode(value, command.gray_img.compress_mode)) return false;
    } else if (key.Equal("--gray-img-quality")) {
      if (!ParseImgQuality(value, command.gray_img.quality)) return false;
    } else if (key.Equal("--mono-img-dpi")) {
      if (!ParseIntValue(value, command.mono_img.dpi)) return false;
    } else if (key.Equal("--mono-img-dpi-limit")) {
      if (!ParseIntValue(value, command.mono_img.dpi_limit)) return false;
    } else if (key.Equal("--mono-img-compress")) {
      if (!ParseMonoCompressMode(value, command.mono_img.compress_mode)) return false;
    } else if (key.Equal("--mono-img-quality")) {
      if (!ParseMonoQuality(value, command.mono_img.quality)) return false;
    } else if (key.Equal("--font-subset-all")) {
      bool b; if (!ParseBoolValue(value, b)) return false;
      command.font_subset_all = b ? 1 : 0;
    } else {
      return false;
    }
  }

  return command.has_input && command.has_output && command.has_options;
}

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

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

class Optimization_Pause : public PauseCallback
{
public:
    Optimization_Pause(int pause_count_limit = 0, bool always_pause = false)
        :pause_count_(0)
        ,pause_count_limit_(pause_count_limit)
        ,always_pause_(always_pause)
    {

    }

    virtual FX_BOOL NeedToPauseNow()
    {
        if (always_pause_) return true;
        if (pause_count_< pause_count_limit_)
        {
            pause_count_ ++;
            return 1;
        }
        else{
            pause_count_ = 0;
            return 0; // This is to test a case: valid PauseCallback but needParseNow() will always return FALSE.
        }
    }

    void  ClearCount()
    {
        pause_count_ = 0;
    }

private:
    int pause_count_limit_;
    int pause_count_;
    bool always_pause_;
};


int main(int argc, char *argv[])
{
  if (argc == 2 && String(argv[1]).Equal("--help")) {
    PrintUsage();
    return 0;
  }

  OptimizationCommand command;
  if (!AnalysisParameter(argc, argv, command)) {
    PrintUsage();
    return 1;
  }

  if (String::FromUnicode(command.input_file).Equal(String::FromUnicode(command.output_file))) {
    cout << "Input path and output path cannot be the same." << endl;
    return 1;
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }
  cout << "Optimization Start." << endl;
  try
  {
      PDFDoc doc(command.input_file);
      ErrorCode error_code = doc.Load();
      if (error_code != foxit::e_ErrSuccess) {
          printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(command.input_file), error_code);
          return 1;
      }
      Optimization_Pause pause(0, true);
      addon::optimization::OptimizerSettings settings;
      settings.SetOptimizerOptions(command.optimizer_options);
      if (command.has_cleanup_options) {
        settings.SetCleanUpOptions(command.cleanup_options);
      }
      if (command.has_discard_objects_options) {
        settings.SetDiscardObjectsOptions(command.discard_objects_options);
      }
      if (command.has_discard_userdata_options) {
        settings.SetDiscardUserDataOptions(static_cast<foxit::uint32>(command.discard_userdata_options));
      }
      if (command.transparency_mode >= 0) {
        settings.SetTransparencyMode(static_cast<OptimizerSettings::TransparencyMode>(command.transparency_mode));
      }
      if (command.color_img.HasAny()) {
        ImageSettings img_s;
        if (command.color_img.dpi >= 0)          img_s.SetImageDPI(command.color_img.dpi);
        if (command.color_img.dpi_limit >= 0)    img_s.SetImageDPILimit(command.color_img.dpi_limit);
        if (command.color_img.compress_mode >= 0) img_s.SetCompressionMode(static_cast<ImageSettings::ImageCompressMode>(command.color_img.compress_mode));
        if (command.color_img.quality >= 0)      img_s.SetQuality(static_cast<ImageSettings::ImageCompressQuality>(command.color_img.quality));
        settings.SetColorImageSettings(img_s);
      }
      if (command.gray_img.HasAny()) {
        ImageSettings img_s;
        if (command.gray_img.dpi >= 0)           img_s.SetImageDPI(command.gray_img.dpi);
        if (command.gray_img.dpi_limit >= 0)     img_s.SetImageDPILimit(command.gray_img.dpi_limit);
        if (command.gray_img.compress_mode >= 0) img_s.SetCompressionMode(static_cast<ImageSettings::ImageCompressMode>(command.gray_img.compress_mode));
        if (command.gray_img.quality >= 0)       img_s.SetQuality(static_cast<ImageSettings::ImageCompressQuality>(command.gray_img.quality));
        settings.SetGrayscaleImageSettings(img_s);
      }
      if (command.mono_img.HasAny()) {
        MonoImageSettings mono_s;
        if (command.mono_img.dpi >= 0)           mono_s.SetImageDPI(command.mono_img.dpi);
        if (command.mono_img.dpi_limit >= 0)     mono_s.SetImageDPILimit(command.mono_img.dpi_limit);
        if (command.mono_img.compress_mode >= 0) mono_s.SetCompressionMode(static_cast<MonoImageSettings::MonoImageCompressMode>(command.mono_img.compress_mode));
        if (command.mono_img.quality >= 0)       mono_s.SetQuality(static_cast<MonoImageSettings::MonoImageCompressQuality>(command.mono_img.quality));
        settings.SetMonoImageSettings(mono_s);
      }
      if (command.font_subset_all >= 0) {
        UnembeddedFontSettings font_s;
        font_s.SetSubsetAllEmFonts(command.font_subset_all == 1);
        settings.SetUnembeddedFontSettings(font_s);
      }

      common::Progressive progressive = addon::optimization::Optimizer::Optimize(doc, settings, &pause);
      Progressive::State progress_state = Progressive::e_ToBeContinued;
      while (Progressive::e_ToBeContinued == progress_state) {
          progress_state = progressive.Continue();
          int percent = progressive.GetRateOfProgress();
          String res_string;
          res_string.Format("Optimize progress percent: %d %", percent);
          std::cout << res_string << std::endl;
      }
      if (Progressive::e_Finished == progress_state)
      {
          doc.SaveAs(command.output_file, foxit::pdf::PDFDoc::e_SaveFlagRemoveRedundantObjects);
      }
  } catch (...) {
    cout << "Unknown Exception" << endl;
    return 1;
  }
  cout << "Optimization Finish." << endl;
  return err_ret;
}