
// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to generate summary for annotation in PDF document.

// Include Foxit SDK header files.
#include <string>
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_annotationsummary.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace annots;

static const char* sn = "";
static const char* key = "";

static std::string g_input_file = "";
static std::string g_output_file = "";
static int g_layout = 1;       // default: e_SummaryLayoutSinglePageWithLine
static int g_sort = 2;         // default: e_TypeSortByPage
static int g_annot_type = 0;   // default: e_UnknownType (all types)
static std::string g_title = "AnnotationSummaryFileTitle";
static bool g_output_no_annot_page = true;
static unsigned int g_line_color = 0xFFFF22;
static float g_line_opacity = 0.5f;

static void PrintHelp() {
  printf(
    "Usage: annotation_summary [options]\n"
    "\n"
    "Required options:\n"
    "  -i <file>             Input PDF file path\n"
    "  -o <file>             Output PDF file path\n"
    "\n"
    "Optional options:\n"
    "  --layout <0-4>        Summary layout (default: 1)\n"
    "                          0: SeparatePagesWithLine\n"
    "                          1: SinglePageWithLine\n"
    "                          2: AnnotationOnly\n"
    "                          3: SeparatePagesWithSequenceNumber\n"
    "                          4: SinglePageWithSequenceNumber\n"
    "  --sort <0-3>          Sort type (default: 2)\n"
    "                          0: ByAuthor  1: ByDate  2: ByPage  3: ByAnnotationType\n"
    "  --annot-type <int>    Annotation type filter, 0=all (default: 0)\n"
    "  --title <string>      File title (default: AnnotationSummaryFileTitle)\n"
    "  --no-annot-page <0|1> Output pages without annotations (default: 1)\n"
    "  --line-color <hex>    Connector line color, e.g. 0xFFFF22 (default: 0xFFFF22)\n"
    "  --line-opacity <f>    Connector line opacity 0.0-1.0 (default: 0.5)\n"
    "  --help                Show this help and exit\n"
  );
}

static bool ParseArgs(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    foxit::String arg(argv[i]);
    if (arg.Equal("--help")) {
      PrintHelp();
      exit(0);
    }
#define NEED_VAL(opt) \
    do { if (i + 1 >= argc) { \
      printf("Missing value for '%s'.\nTry 'annotation_summary --help' for more information.\n", (opt)); \
      return false; \
    } } while(0)

    if (arg.Equal("-i")) {
      NEED_VAL("-i"); ++i;
      g_input_file = argv[i];
    } else if (arg.Equal("-o")) {
      NEED_VAL("-o"); ++i;
      g_output_file = argv[i];
    } else if (arg.Equal("--layout")) {
      NEED_VAL("--layout"); ++i;
      g_layout = atoi(argv[i]);
    } else if (arg.Equal("--sort")) {
      NEED_VAL("--sort"); ++i;
      g_sort = atoi(argv[i]);
    } else if (arg.Equal("--annot-type")) {
      NEED_VAL("--annot-type"); ++i;
      g_annot_type = atoi(argv[i]);
    } else if (arg.Equal("--title")) {
      NEED_VAL("--title"); ++i;
      g_title = argv[i];
    } else if (arg.Equal("--no-annot-page")) {
      NEED_VAL("--no-annot-page"); ++i;
      g_output_no_annot_page = (atoi(argv[i]) != 0);
    } else if (arg.Equal("--line-color")) {
      NEED_VAL("--line-color"); ++i;
      g_line_color = (unsigned int)strtoul(argv[i], NULL, 0);
    } else if (arg.Equal("--line-opacity")) {
      NEED_VAL("--line-opacity"); ++i;
      g_line_opacity = (float)atof(argv[i]);
    } else {
      printf("Unknown argument: '%s'.\nTry 'annotation_summary --help' for more information.\n", argv[i]);
      return false;
    }
#undef NEED_VAL
  }

  if (g_input_file.empty()) {
    printf("Error: -i <file> is required.\nTry 'annotation_summary --help' for more information.\n");
    return false;
  }
  if (g_output_file.empty()) {
    printf("Error: -o <file> is required.\nTry 'annotation_summary --help' for more information.\n");
    return false;
  }
  return true;
}

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

int main(int argc, char *argv[])
{
  if (!ParseArgs(argc, argv)) return 1;

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }
  try {
    WString input_file = WString::FromLocal(g_input_file.c_str());
    WString output_file = WString::FromLocal(g_output_file.c_str());

    // Create output parent directory if needed
    {
      std::size_t osep = g_output_file.find_last_of("/\\");
      if (osep != std::string::npos) {
        std::string out_parent = g_output_file.substr(0, osep);
#if defined(_WIN32) || defined(_WIN64)
        _mkdir(out_parent.c_str());
#else
        mkdir(out_parent.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
      }
    }

    // Load PDF document.
    PDFDoc doc(input_file);
    ErrorCode load_code = doc.Load();
    if (load_code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", g_input_file.c_str(), load_code);
      return 1;
    }
    AnnotationSummary commentssummary = AnnotationSummary(doc);
    AnnotationSummarySettings settings;
    settings.SetSummaryLayout((AnnotationSummarySettings::SummaryLayout)g_layout);
    settings.SetSortType((AnnotationSummarySettings::SortType)g_sort);
    settings.SetAnnotType((annots::Annot::Type)g_annot_type, true);
    settings.SetFileTitle(WString::FromLocal(g_title.c_str()));
    settings.EnableOutputNoAnnotationPage(g_output_no_annot_page);
    settings.SetConnectorLineColor(g_line_color);
    settings.SetConnectorLineOpacity(g_line_opacity);

    commentssummary.StartSummarize(output_file, settings, NULL);

    cout << "Generate annotation's summary to pdf successfully." << endl;
  }
  catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  }
  catch (...)
  {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}
