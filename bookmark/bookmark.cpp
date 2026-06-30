// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to enumerate and modify bookmarks
// in PDF document.

// Include Foxit SDK header files.
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cerrno>
#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_filespec.h"
#include "../../../include/pdf/fs_bookmark.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

static std::string g_action;
static std::string g_input_file;
static std::string g_output_file;
static std::string g_title;
static int g_dest_page = -1;
static bool g_has_title = false;
static bool g_has_dest_page = false;

static void PrintUsage() {
  printf(
    "Usage: demo_bookmark --action <get|add> -i <input.pdf> [options]\n"
    "\n"
    "Required options:\n"
    "  --action <name>       get | add\n"
    "  -i <file>             Input PDF path\n"
    "\n"
    "Write actions required option:\n"
    "  -o <file>             Output PDF path (required for add)\n"
    "\n"
    "Action options:\n"
    "  --title <text>        Bookmark title for add\n"
    "  --dest-page <n>       Destination page index for add\n"
    "\n"
    "Examples:\n"
    "  demo_bookmark --action get -i ./input/AboutFoxit.pdf\n"
    "  demo_bookmark --action add -i ./input/AboutFoxit.pdf -o ./output/add.pdf --title \"New Bookmark\" --dest-page 0\n"
  );
}

static bool ParseIntArg(const char* value, int* out_value) {
  char* end = NULL;
  errno = 0;
  long parsed = strtol(value, &end, 10);
  if (errno != 0 || end == value || *end != '\0') {
    return false;
  }
  *out_value = static_cast<int>(parsed);
  return true;
}

static bool IsWriteAction() {
  return g_action == "add";
}

static bool ParseArgs(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    String arg(argv[i]);
    if (arg.Equal("--help")) {
      PrintUsage();
      exit(0);
    }

    if (arg.Equal("--action")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--action'.\n");
        return false;
      }
      g_action = argv[++i];
    } else if (arg.Equal("-i")) {
      if (i + 1 >= argc) {
        printf("Missing value for '-i'.\n");
        return false;
      }
      g_input_file = argv[++i];
    } else if (arg.Equal("-o")) {
      if (i + 1 >= argc) {
        printf("Missing value for '-o'.\n");
        return false;
      }
      g_output_file = argv[++i];
    } else if (arg.Equal("--title")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--title'.\n");
        return false;
      }
      g_title = argv[++i];
      g_has_title = true;
    } else if (arg.Equal("--dest-page")) {
      if (i + 1 >= argc) {
        printf("Missing value for '--dest-page'.\n");
        return false;
      }
      if (!ParseIntArg(argv[++i], &g_dest_page)) {
        printf("Invalid value for '--dest-page': %s\n", argv[i]);
        return false;
      }
      g_has_dest_page = true;
    } else {
      printf("Unknown argument: %s\n", argv[i]);
      return false;
    }
  }
  return true;
}

static bool ValidateArgs() {
  if (g_action.empty()) {
    printf("Error: --action is required.\n");
    return false;
  }
  if (!(g_action == "get" || g_action == "add")) {
    printf("Error: invalid --action value '%s'.\n", g_action.c_str());
    return false;
  }
  if (g_input_file.empty()) {
    printf("Error: -i is required.\n");
    return false;
  }
  if (IsWriteAction() && g_output_file.empty()) {
    printf("Error: -o is required for write actions (add).\n");
    return false;
  }

  if (g_has_dest_page && g_dest_page < 0) {
    printf("Error: --dest-page must be greater than or equal to 0.\n");
    return false;
  }

  if (g_action == "add") {
    if (!g_has_title) {
      printf("Error: --title is required for add action.\n");
      return false;
    }
    if (!g_has_dest_page) {
      printf("Error: --dest-page is required for add action.\n");
      return false;
    }
  }

  std::ifstream input_test(g_input_file.c_str(), std::ios::binary);
  if (!input_test.good()) {
    printf("Error: input file does not exist or cannot be opened: %s\n", g_input_file.c_str());
    return false;
  }
  return true;
}

static std::string GetParentDir(const std::string& path) {
  size_t pos = path.find_last_of("/\\");
  if (pos == std::string::npos) return "";
  return path.substr(0, pos);
}

static void EnsureOutputDir(const std::string& dir_path) {
  if (dir_path.empty()) return;

  std::string normalized = dir_path;
  for (size_t i = 0; i < normalized.size(); ++i) {
    if (normalized[i] == '\\') normalized[i] = '/';
  }

  std::string current;
  size_t start = 0;
  if (normalized.size() > 1 && normalized[1] == ':') {
    current = normalized.substr(0, 2);
    start = 2;
  }
  if (start < normalized.size() && normalized[start] == '/') {
    current += '/';
    ++start;
  }

  while (start < normalized.size()) {
    size_t sep = normalized.find('/', start);
    std::string token = (sep == std::string::npos)
      ? normalized.substr(start)
      : normalized.substr(start, sep - start);

    if (!token.empty()) {
      if (!current.empty() && current[current.size() - 1] != '/') current += '/';
      current += token;
#if defined(_WIN32) || defined(_WIN64)
      _mkdir(current.c_str());
#else
      mkdir(current.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
    }

    if (sep == std::string::npos) break;
    start = sep + 1;
  }
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

static void PrintBookmarkByPath(const Bookmark& bookmark, const std::string& path, int depth) {
  std::string indent(depth * 2, ' ');
  printf("%s[%s] title=%s color=0x%06X\n",
    indent.c_str(),
    path.c_str(),
    (const char*)String::FromUnicode(bookmark.GetTitle()),
    bookmark.GetColor());
}

static void PrintBookmarkTree(const Bookmark& node, std::vector<int>& path, int depth) {
  if (node.IsEmpty()) return;

  std::string path_str;
  for (size_t i = 0; i < path.size(); ++i) {
    if (i > 0) path_str += "/";
    path_str += std::to_string(path[i]);
  }
  PrintBookmarkByPath(node, path_str, depth);

  Bookmark child = ((Bookmark)node).GetFirstChild();
  int child_index = 0;
  while (!child.IsEmpty()) {
    path.push_back(child_index);
    PrintBookmarkTree(child, path, depth + 1);
    path.pop_back();
    child = child.GetNextSibling();
    ++child_index;
  }
}

static bool HandleGet(const Bookmark& root) {
  if (root.IsEmpty() || ((Bookmark)root).GetFirstChild().IsEmpty()) {
    printf("No bookmarks found.\n");
    return true;
  }

  Bookmark top = ((Bookmark)root).GetFirstChild();
  int top_index = 0;
  while (!top.IsEmpty()) {
    std::vector<int> path;
    path.push_back(top_index);
    PrintBookmarkTree(top, path, 0);
    top = top.GetNextSibling();
    ++top_index;
  }
  return true;
}

static bool HandleAdd(PDFDoc& doc, Bookmark& root) {
  if (root.IsEmpty()) {
    root = doc.CreateRootBookmark();
  }
  if (root.IsEmpty()) {
    printf("Error: failed to create bookmark root.\n");
    return false;
  }

  Bookmark added = root.Insert(WString::FromLocal(g_title.c_str()), Bookmark::e_PosLastChild);
  if (added.IsEmpty()) {
    printf("Error: failed to add bookmark.\n");
    return false;
  }
  Destination dest = Destination::CreateFitPage(doc, g_dest_page);
  added.SetDestination(dest);
  printf("Added bookmark: title='%s', dest-page=%d\n", g_title.c_str(), g_dest_page);
  return true;
}

int main(int argc, char *argv[]) {
  if (!ParseArgs(argc, argv) || !ValidateArgs()) {
    PrintUsage();
    return 1;
  }

  int err_ret = 0;

  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    WString input_file = WString::FromLocal(g_input_file.c_str());
    PDFDoc doc(input_file);
    error_code = doc.Load();
    if (error_code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", g_input_file.c_str(), error_code);
      return 1;
    }

    if (g_has_dest_page && g_dest_page >= doc.GetPageCount()) {
      printf("Error: --dest-page %d is out of range [0, %d].\n", g_dest_page, doc.GetPageCount() - 1);
      return 1;
    }

    Bookmark root = doc.GetRootBookmark();
    bool ok = false;
    if (g_action == "get") {
      ok = HandleGet(root);
    } else if (g_action == "add") {
      ok = HandleAdd(doc, root);
    }

    if (!ok) {
      return 1;
    }

    if (IsWriteAction()) {
      EnsureOutputDir(GetParentDir(g_output_file));
      WString output_file = WString::FromLocal(g_output_file.c_str());
      doc.SaveAs(output_file, PDFDoc::e_SaveFlagNoOriginal);
      printf("Saved output PDF: %s\n", g_output_file.c_str());
    }
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  } catch (...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}

