// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to create a PDF document,
// insert the text path and image objects into the created PDF document, copy shading objects,
// and save the file with inserted graphics objects.

// Include Foxit SDK header files.
#include <time.h>
#include <iostream>
#include <cstdio>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif
#include <set>
#include <cctype>

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/graphics/fs_pdfgraphicsobject.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace graphics;

#define DEMOMSG       printf("Please make sure the key %s is valid and it has value.\n", (FX_LPCSTR)argv_key); \
                          printf("Usage: graphics_objects -i <input pdf path> -o <output pdf path> --op <add|remove> --object-types <text|image|path> [--image <image file path>]\nPlease try 'graphics_objects --help' for more information.\n");

enum OperationMode {
  e_OpInvalid = 0,
  e_OpAdd,
  e_OpRemove,
};

struct CliOptions {
  WString input_file;
  WString output_file;
  OperationMode operation;
  String object_types_raw;
  WString image_file;
  bool show_help;

  CliOptions() : operation(e_OpInvalid), show_help(false) {}
};

void PrintUsage() {
  cout << "Usage:" << endl
       << "graphics_objects -i <input pdf path> -o <output pdf path> --op <add|remove> --object-types <text|image|path> [--image <image file path>]" << endl
       << endl
       << "Required:" << endl
       << "  -i <path>                Input PDF path" << endl
       << "  -o <path>                Output PDF path" << endl
       << "  --op <mode>              Operation mode: add or remove" << endl
       << "  --object-types <types>   Comma-separated object types to add: text, image, path" << endl
       << endl
       << "Optional:" << endl
       << "  --image <path>           Image file path for image object (default: sdk.png from input_files)" << endl
       << "  --help                   Show this help message" << endl;
}

OperationMode ParseOperation(const String& op) {
  if (op.Equal("add")) return e_OpAdd;
  if (op.Equal("remove")) return e_OpRemove;
  return e_OpInvalid;
}

bool ParseArgs(int argc, char* argv[], CliOptions& options) {
  for (int i = 1; i < argc; ++i) {
    String argv_key = String(argv[i]);
    if (argv_key.Equal("--help")) {
      options.show_help = true;
      return true;
    }

    if (argc <= i + 1) {
      DEMOMSG
      return false;
    }

    String argv_value = String(argv[++i]);
    if (argv_key.Equal("-i")) options.input_file = WString::FromUTF8(argv_value);
    else if (argv_key.Equal("-o")) options.output_file = WString::FromUTF8(argv_value);
    else if (argv_key.Equal("--op")) {
      options.operation = ParseOperation(argv_value);
      if (options.operation == e_OpInvalid) {
        cout << "Invalid value for --op: " << argv_value << endl;
        return false;
      }
    }
    else if (argv_key.Equal("--object-types")) {
      options.object_types_raw = argv_value;
    }
    else if (argv_key.Equal("--image")) {
      options.image_file = WString::FromUTF8(argv_value);
    }
    else {
      DEMOMSG
      return false;
    }
  }
  return true;
}

bool ValidateArgs(const CliOptions& options) {
  if (options.show_help) return true;
  if (options.input_file.IsEmpty()) {
    cout << "Missing required parameter: -i" << endl;
    return false;
  }
  if (options.output_file.IsEmpty()) {
    cout << "Missing required parameter: -o" << endl;
    return false;
  }
  if (options.operation == e_OpInvalid) {
    cout << "Missing required parameter: --op" << endl;
    return false;
  }
  if (options.operation == e_OpAdd && options.object_types_raw.IsEmpty()) {
    cout << "Missing required parameter: --object-types (required for add mode)" << endl;
    return false;
  }
  return true;
}

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

void AddTextObjects(PDFPage page) {
  POSITION position = page.GetLastGraphicsObjectPosition(GraphicsObject::e_TypeText);
  TextObject* text_object = TextObject::Create();

  text_object->SetFillColor(0xFFFF7F00);

  // Prepare text state
  TextState state;
  state.font_size = 80.0f;
  state.font =  Font(L"Simsun", Font::e_StylesSmallCap, Font::e_CharsetGB2312, 0);
  state.textmode = TextState::e_ModeFill;
  text_object->SetTextState(page, state, false, 750);

  // Set text.
  text_object->SetText(L"Foxit Software");
  POSITION last_position = page.InsertGraphicsObject(position, text_object);

  RectF rect = text_object->GetRect();
  float offset_x = (page.GetWidth() - (rect.right - rect.left)) / 2;
  float offset_y = page.GetHeight() * 0.8f - (rect.top - rect.bottom) / 2;
  text_object->Transform(Matrix(1, 0, 0, 1, offset_x, offset_y), false);

  // Generate content
  page.GenerateContent();

  // Clone a text object from the old text object.
  text_object = reinterpret_cast<TextObject*>(text_object->Clone());

  state.font = Font(Font::e_StdIDTimes);
  state.font_size = 48;
  state.textmode = TextState::e_ModeFillStrokeClip;

  text_object->SetTextState(page, state, true, 750);
  text_object->SetText(L"www.foxitsoftware.com");

  text_object->SetFillColor(0xFFAAAAAA);
  text_object->SetStrokeColor(0xFFF68C21);
  page.InsertGraphicsObject(last_position, text_object);

  rect = text_object->GetRect();
  offset_x = (page.GetWidth() - (rect.right - rect.left)) / 2;
  offset_y = page.GetHeight() * 0.618f - (rect.top - rect.bottom) / 2;
  text_object->Transform(Matrix(1, 0, 0, 1, offset_x, offset_y), false);

  // Generate content again after transformation.
  page.GenerateContent();
}

void AddImageObjects(PDFPage page, WString image_file) {
  POSITION position = page.GetLastGraphicsObjectPosition(GraphicsObject::e_TypeImage);
  Image image(image_file);
  ImageObject* image_object = ImageObject::Create(page.GetDocument());
  image_object->SetImage(image, 0);

  float width = static_cast<float>(image.GetWidth());
  float height = static_cast<float>(image.GetHeight());

  float page_width = page.GetWidth();
  float page_height = page.GetHeight();

  // Please notice the matrix value.
  image_object->SetMatrix(Matrix(width, 0, 0, height, (page_width - width) / 2.0f, (page_height - height) / 2.0f));

  page.InsertGraphicsObject(position, image_object);
  page.GenerateContent();
}

void AddPieces(PDFPage page, ShadingObject* orignal_pieces, const RectF& dst_rect) {
  POSITION position = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeAll);
  ShadingObject* pieces = (ShadingObject*)orignal_pieces->Clone();

  RectF piece_rect = pieces->GetRect();

  // Calculates the transformation matrix between dst_rect and  piece_rect.
  float a = (dst_rect.right - dst_rect.left) / (piece_rect.right - piece_rect.left);
  float d = (dst_rect.top - dst_rect.bottom) / (piece_rect.top - piece_rect.bottom);
  float e = dst_rect.left - piece_rect.left * a;
  float f = dst_rect.top - piece_rect.top * d;

  // Transform rect.
  pieces->Transform(Matrix(a, 0, 0, d, e, f), false);
  page.InsertGraphicsObject(position, pieces);

  page.GenerateContent();
}

void AddPathObjects(PDFPage page, ShadingObject* black_pieces, ShadingObject* white_pieces) {
  POSITION position = page.GetLastGraphicsObjectPosition(GraphicsObject::e_TypePath);
  PathObject* path_object = PathObject::Create();
  Path path;
  float page_width = page.GetWidth();
  float page_height = page.GetHeight();

  float width = min(page_width, page_height) / 20.0f;
  float start_x = (page_width - width * 18.0f) / 2.0f;
  float start_y = (page_height - width * 18.0f) / 2.0f;

  // Draw a chess board
  for (int i = 0; i < 19; i++) {
    float x1 = start_x;
    float y1 = i * width + start_y;

    float x2 = start_x + 18 * width;
    path.MoveTo(PointF(x1, y1));
    path.LineTo(PointF(x2, y1));

    x1 = i * width + start_x;
    y1 = start_y;

    float y2 = 18 * width + start_y;
    path.MoveTo(PointF(x1, y1));
    path.LineTo(PointF(x1, y2));
  }

  int star[3] = {3, 9, 15};
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      RectF rect(start_x + star[i] * width - width / 12, start_y + star[j] * width - width / 12,
        start_x + star[i] * width + width / 12, start_y + star[j] * width + width / 12);
      path.AppendEllipse(rect);
    }
  }
  path_object->SetPathData(path);

  path_object->SetFillColor(0xFF000000);
  path_object->SetFillMode(e_FillModeAlternate);
  path_object->SetStrokeState(true);
  path_object->SetStrokeColor(0xFF000000);

  page.InsertGraphicsObject(position, path_object);
  page.GenerateContent();

  // Draw pieces
  PointF pieces_vector[2][8] = {{PointF(3, 3),   PointF(3, 7),   PointF(3, 15),  PointF(13, 2),
    PointF(13, 16), PointF(13, 17), PointF(15, 16), PointF(16, 16)},
  {PointF(11, 16), PointF(12, 14), PointF(14, 4),  PointF(14, 15),
  PointF(15, 3),  PointF(15, 9),  PointF(15, 15), PointF(16, 15)}};
  for (int k = 0; k < 2; k++) {
    for (int i = 0; i < sizeof(pieces_vector[k]) / sizeof(pieces_vector[k][0]); i++) {
      int x = static_cast<int>(pieces_vector[k][i].x);
      int y = static_cast<int>(pieces_vector[k][i].y);
      AddPieces(page, k % 2 ? white_pieces : black_pieces,
        RectF(start_x + x * width - width / 2.f, start_y + y * width - width / 2.f,
        start_x + x * width + width / 2.f, start_y + y * width + width / 2.f));
    }
  }
}

void RemoveTextObjects(PDFPage page) {
  page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
  POSITION position = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeAll);
  while (position) {
    GraphicsObject* obj = (GraphicsObject*)page.GetGraphicsObject(position);
    if (obj->GetType() == GraphicsObject::e_TypeText) {
      TextObject *textobj = (TextObject *)obj;
      WString s = textobj->GetText();
      if (s.GetLength() > 0) {
        page.RemoveGraphicsObject(textobj);
        position = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeAll);
        continue;
      }
    } else if (obj->GetType() == GraphicsObject::e_TypeFormXObject) {
      FormXObject *formxobj = (FormXObject *)obj;
      GraphicsObjects graphicsObjects_form = formxobj->GetGraphicsObjects();
      foxit::POSITION pos = graphicsObjects_form.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeText);
      if (pos) {
        TextObject* textobj = (TextObject*)page.GetGraphicsObject(pos);
        if (textobj->GetType() == GraphicsObject::e_TypeText) {
          WString s = textobj->GetText();
          if (s.GetLength() > 0) {
            page.RemoveGraphicsObject(obj);
            position = page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeAll);
            continue;
          }
        }
      }
    }
    position = page.GetNextGraphicsObjectPosition(position, GraphicsObject::e_TypeAll);
  }
  page.GenerateContent();
}

int main(int argc, char *argv[])
{
  CliOptions options;
  if (!ParseArgs(argc, argv, options)) {
    PrintUsage();
    return 1;
  }
  if (options.show_help) {
    PrintUsage();
    return 0;
  }
  if (!ValidateArgs(options)) {
    PrintUsage();
    return 1;
  }

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    PDFDoc doc(options.input_file);
    ErrorCode error_code = doc.Load();

    if (error_code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(options.input_file), error_code);
      return 1;
    }

    if (options.operation == e_OpAdd) {
      // Parse object types
      std::vector<std::string> object_types;
      std::string types_text = std::string((const char*)options.object_types_raw);
      std::set<std::string> used;
      size_t start = 0;
      while (start <= types_text.size()) {
        size_t comma = types_text.find(',', start);
        std::string token = (comma == std::string::npos) ? types_text.substr(start) : types_text.substr(start, comma - start);
        // Trim whitespace
        size_t b = 0;
        while (b < token.size() && isspace((unsigned char)token[b])) ++b;
        size_t e = token.size();
        while (e > b && isspace((unsigned char)token[e - 1])) --e;
        token = token.substr(b, e - b);
        if (!token.empty()) {
          std::string lower = token;
          std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return (char)std::tolower(c); });
          if (lower != "text" && lower != "image" && lower != "path") {
            cout << "Invalid object type: " << token << ". Expected: text, image, path." << endl;
            return 1;
          }
          if (used.find(lower) == used.end()) {
            used.insert(lower);
            object_types.push_back(lower);
          }
        }
        if (comma == std::string::npos) break;
        start = comma + 1;
      }
      if (object_types.empty()) {
        cout << "--object-types is empty. Please provide at least one type." << endl;
        return 1;
      }

      PDFPage original_page = doc.GetPage(0);
      original_page.StartParse(PDFPage::e_ParsePageNormal, NULL, false);
      POSITION position = original_page.GetFirstGraphicsObjectPosition(GraphicsObject::e_TypeShading);
      if (!position) return 1;
      ShadingObject* black_pieces = (ShadingObject*)original_page.GetGraphicsObject(position);
      position = original_page.GetNextGraphicsObjectPosition(position, GraphicsObject::e_TypeShading);
      ShadingObject* white_pieces = (ShadingObject*)original_page.GetGraphicsObject(position);

      int page_index = 0;
      for (size_t i = 0; i < object_types.size(); ++i) {
        const std::string& type = object_types[i];
        PDFPage page = doc.InsertPage(page_index);
        if (type == "text") {
          AddTextObjects(page);
        } else if (type == "image") {
          WString image_file = options.image_file.IsEmpty() ? (input_path + L"sdk.png") : options.image_file;
          AddImageObjects(page, image_file);
        } else if (type == "path") {
          AddPathObjects(page, black_pieces, white_pieces);
        }
        page_index++;
      }
      cout << "Add graphics objects action completed." << endl;
    }

    if (options.operation == e_OpRemove) {
      PDFPage page = doc.GetPage(0);
      RemoveTextObjects(page);
      cout << "Remove graphics objects action completed." << endl;
    }

    doc.SaveAs(options.output_file, PDFDoc::e_SaveFlagNormal);
    cout << "graphics_objects command finished." << endl;

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
