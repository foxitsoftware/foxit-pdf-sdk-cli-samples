// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to render all PDF layers of a PDF,
// and add layers in a PDF document.

// Include Foxit SDK header files.
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
#include "../../../include/pdf/fs_pdflayer.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

struct PdfLayerCommand {
  WString input_file;
  WString output_file;
  WString action;           // query, add, modify, remove, move
  WString layer_name;       // for add: new layer name; for modify: target layer name
  bool layer_has_layer;     // for add: whether new node has layer
  int node_index;           // for remove/move: child index; for add: insert position
  int target_index;         // for move: destination child index
  WString target_parent;    // for move: destination parent layer name (empty = root)
  bool visible;             // for modify: default visibility
  WString view_usage;       // for modify: on/off/undefined
  WString new_name;         // for modify: rename layer
  bool show_help;

  PdfLayerCommand()
      : layer_has_layer(true),
        node_index(-1),
        target_index(0),
        visible(true),
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
  cout << "pdflayer --action <query|add|modify|remove|move> --input <input.pdf> --output <output.pdf> [options]" << endl << endl;
  cout << "Required:" << endl;
  cout << "  --action <type>                 Action: query | add | modify | remove | move." << endl;
  cout << "  --input <path>                  Input PDF file path." << endl;
  cout << "  --output <path>                 Output PDF file path (or output txt for query)." << endl << endl;
  cout << "Add options:" << endl;
  cout << "  --layer-name <name>             New layer node name." << endl;
  cout << "  --has-layer <on|off>            Whether new node has layer. Default: on." << endl;
  cout << "  --node-index <int>              Insert position (0-based). Default: append." << endl << endl;
  cout << "Modify options:" << endl;
  cout << "  --layer-name <name>             Target layer node name to modify." << endl;
  cout << "  --visible <on|off>              Set default visibility." << endl;
  cout << "  --view-usage <on|off|undefined> Set view usage state." << endl;
  cout << "  --new-name <name>               Rename the layer node." << endl << endl;
  cout << "Remove options:" << endl;
  cout << "  --node-index <int>              Child index to remove (0-based)." << endl << endl;
  cout << "Move options:" << endl;
  cout << "  --node-index <int>              Source child index to move (0-based)." << endl;
  cout << "  --target-parent <name>          Destination parent layer name (empty = root)." << endl;
  cout << "  --target-index <int>            Destination child index. Default: 0." << endl << endl;
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

bool ParseCommand(int argc, char* argv[], PdfLayerCommand& command) {
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
      if (!value.Equal("query") && !value.Equal("add") && !value.Equal("modify") &&
          !value.Equal("remove") && !value.Equal("move")) {
        printf("Invalid action: %s (must be query, add, modify, remove, or move)\n", (const char*)value);
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
    } else if (key.Equal("--layer-name")) {
      command.layer_name = WString::FromUTF8(value);
    } else if (key.Equal("--has-layer")) {
      command.layer_has_layer = value.Equal("on") || value.Equal("true") || value.Equal("1");
    } else if (key.Equal("--node-index")) {
      command.node_index = atoi((const char*)value);
    } else if (key.Equal("--target-parent")) {
      command.target_parent = WString::FromUTF8(value);
    } else if (key.Equal("--target-index")) {
      command.target_index = atoi((const char*)value);
    } else if (key.Equal("--visible")) {
      command.visible = value.Equal("on") || value.Equal("true") || value.Equal("1");
    } else if (key.Equal("--view-usage")) {
      command.view_usage = WString::FromUTF8(value);
    } else if (key.Equal("--new-name")) {
      command.new_name = WString::FromUTF8(value);
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
  return true;
}

class TextDoc {
public:
  TextDoc(const String& file_name, const String& fill_mode);
  TextDoc(const WString& file_name, const WString& fill_mode);
  ~TextDoc();
  void Write(const char* format, ...);
  void Write(int count, const char * prefix, const char * format, ...);

private:
  FILE* file_;
};

TextDoc::TextDoc(const String& file_name, const String& file_mode) throw(Exception) : file_(NULL) {
#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file_, file_name, (const char*)file_mode);
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

String UsgaeCodeToString(LayerTree::UsageState state) {
  switch (state) {
  case LayerTree::e_StateON:
    return "ON";
  case LayerTree::e_StateOFF:
    return "OFF";
  case LayerTree::e_StateUnchanged:
    return "Unchanged";
  case LayerTree::e_StateUndefined:
    return "Undefined";
  }
  return "Unknown";
}

void GetAllLayerNodesInformation(LayerNode layer_node, int depth, TextDoc& text_doc) {

  if (depth >= 0) {
    text_doc.Write(depth, "\t", "%s", (const char*)String::FromUnicode(layer_node.GetName()));
    if (layer_node.HasLayer()) {
      LayerTree::UsageState state = layer_node.GetViewUsage();
      text_doc.Write(" %s\r\n", state == LayerTree::e_StateON ? "[*]" : "[ ]");
      text_doc.Write(depth, "\t", "View usage state:\t%s\r\n", (const char*)UsgaeCodeToString(state));
      text_doc.Write(depth, "\t", "Export usage state:\t%s\r\n", (const char*)UsgaeCodeToString(layer_node.GetExportUsage()));

      LayerPrintData print_data = layer_node.GetPrintUsage();
      text_doc.Write(depth, "\t", "Print usage state:\t%s, subtype: %s\r\n", (const char*)UsgaeCodeToString(print_data.print_state),
        (const char*)print_data.subtype);

      LayerZoomData zoom_data = layer_node.GetZoomUsage();
      text_doc.Write(depth, "\t", "Zoom usage:\tmin_factor = %.4f max_factor = %.4f\r\n\r\n", zoom_data.min_factor,
        zoom_data.max_factor);
    } else {
      text_doc.Write("\r\n");
    }
  }

  depth++;
  int count = layer_node.GetChildrenCount();
  for (int i = 0; i < count; i++) {
    LayerNode child = layer_node.GetChild(i);
    GetAllLayerNodesInformation(child, depth, text_doc);
  }
}

LayerNode FindLayerNodeByName(LayerNode parent, const WString& name) {
  if (parent.HasLayer() && parent.GetName() == name) {
    return parent;
  }
  int count = parent.GetChildrenCount();
  for (int i = 0; i < count; i++) {
    LayerNode found = FindLayerNodeByName(parent.GetChild(i), name);
    if (!found.IsEmpty()) return found;
  }
  return LayerNode();
}

int main(int argc, char* argv[]) {
  PdfLayerCommand command;
  if (!ParseCommand(argc, argv, command)) return 1;

  SdkLibMgr sdk_lib_mgr;
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) return 1;

  // Derive output directory from output file path
  WString output_dir;
  String output_str = String::FromUnicode(command.output_file);
  int last_sep = output_str.ReverseFind('/');
#if defined(_WIN32) || defined(_WIN64)
  int last_sep_win = output_str.ReverseFind('\\');
  if (last_sep_win > last_sep) last_sep = last_sep_win;
#endif
  if (last_sep >= 0) {
    output_dir = WString::FromUTF8(output_str.Mid(0, last_sep + 1));
#if defined(_WIN32) || defined(_WIN64)
    _mkdir(String::FromUnicode(output_dir));
#else
    mkdir(String::FromUnicode(output_dir), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
  }

  try {
    PDFDoc doc(command.input_file);
    error_code = doc.Load();
    if (error_code != foxit::e_ErrSuccess) {
      printf("Error: Load PDF \"%s\" failed. Error code: %d\n",
             (const char*)String::FromUnicode(command.input_file), error_code);
      return 1;
    }

    LayerTree layertree(doc);
    LayerNode root = layertree.GetRootNode();
    if (root.IsEmpty()) {
      printf("No layer information in the document.\n");
      return 1;
    }

    String action = String::FromUnicode(command.action);

    if (action.Equal("query")) {
      // Output layer information to text file
      TextDoc info_doc(command.output_file, L"w+b");
      GetAllLayerNodesInformation(root, -1, info_doc);
      cout << "Layer information written to: " << (const char*)String::FromUnicode(command.output_file) << endl;

    } else if (action.Equal("add")) {
      if (command.layer_name.IsEmpty()) {
        printf("Error: --layer-name is required for add action.\n");
        return 1;
      }
      int insert_index = (command.node_index >= 0) ? command.node_index : root.GetChildrenCount();
      root.AddChild(insert_index, (const wchar_t*)command.layer_name, command.layer_has_layer);
      doc.SaveAs(command.output_file, PDFDoc::e_SaveFlagNormal);
      cout << "Layer node added. Output: " << (const char*)String::FromUnicode(command.output_file) << endl;

    } else if (action.Equal("modify")) {
      if (command.layer_name.IsEmpty()) {
        printf("Error: --layer-name is required for modify action.\n");
        return 1;
      }
      LayerNode target = FindLayerNodeByName(root, command.layer_name);
      if (target.IsEmpty()) {
        printf("Error: Layer node \"%s\" not found.\n", (const char*)String::FromUnicode(command.layer_name));
        return 1;
      }
      if (!command.new_name.IsEmpty()) {
        target.SetName((const wchar_t*)command.new_name);
      }
      if (!command.view_usage.IsEmpty()) {
        String vu = String::FromUnicode(command.view_usage);
        if (vu.Equal("on")) target.SetViewUsage(LayerTree::e_StateON);
        else if (vu.Equal("off")) target.SetViewUsage(LayerTree::e_StateOFF);
        else if (vu.Equal("undefined")) target.SetViewUsage(LayerTree::e_StateUndefined);
      }
      target.SetDefaultVisible(command.visible);
      doc.SaveAs(command.output_file, PDFDoc::e_SaveFlagNormal);
      cout << "Layer node modified. Output: " << (const char*)String::FromUnicode(command.output_file) << endl;

    } else if (action.Equal("remove")) {
      if (command.node_index < 0) {
        printf("Error: --node-index is required for remove action.\n");
        return 1;
      }
      if (!root.RemoveChild(command.node_index)) {
        printf("Error: Failed to remove child at index %d.\n", command.node_index);
        return 1;
      }
      doc.SaveAs(command.output_file, PDFDoc::e_SaveFlagNormal);
      cout << "Layer node removed. Output: " << (const char*)String::FromUnicode(command.output_file) << endl;

    } else if (action.Equal("move")) {
      if (command.node_index < 0) {
        printf("Error: --node-index is required for move action.\n");
        return 1;
      }
      LayerNode child_to_move = root.GetChild(command.node_index);
      if (child_to_move.IsEmpty()) {
        printf("Error: Child at index %d not found.\n", command.node_index);
        return 1;
      }
      LayerNode dest_parent = root;
      if (!command.target_parent.IsEmpty()) {
        dest_parent = FindLayerNodeByName(root, command.target_parent);
        if (dest_parent.IsEmpty()) {
          printf("Error: Target parent layer \"%s\" not found.\n", (const char*)String::FromUnicode(command.target_parent));
          return 1;
        }
      }
      if (!child_to_move.MoveTo(dest_parent, command.target_index)) {
        printf("Error: Failed to move layer node.\n");
        return 1;
      }
      doc.SaveAs(command.output_file, PDFDoc::e_SaveFlagNormal);
      cout << "Layer node moved. Output: " << (const char*)String::FromUnicode(command.output_file) << endl;
    }
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    return 1;
  }
  catch(...) {
    cout << "Unknown Exception" << endl;
    return 1;
  }

  return 0;
}
