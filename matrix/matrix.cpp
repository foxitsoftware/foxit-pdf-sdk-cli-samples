// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file is a demo to demonstrate how to use matrix to translate, scale... objects.

// Include Foxit SDK header files.
#include <time.h>
#include <cstdlib>
#include <iostream>

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/objects/fs_pdfobject.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;

enum TransformOp {
  kTransformUnknown = 0,
  kTransformTranslate,
  kTransformRotate,
  kTransformScale,
  kTransformShear
};

enum ObjectTypeFilter {
  kObjectTypeText = 0,
  kObjectTypeImage,
  kObjectTypeBoth
};

struct TransformCommand {
  TransformOp op;
  float tx;
  float ty;
  float angle;
  float sx;
  float sy;
  float kx;
  float ky;
  bool has_tx;
  bool has_ty;
  bool has_angle;
  bool has_sx;
  bool has_sy;
  bool has_kx;
  bool has_ky;
  bool has_op;
  ObjectTypeFilter object_type;
  WString text_content;
  bool has_text_content;

  TransformCommand()
      : op(kTransformUnknown),
        tx(0.0f),
        ty(0.0f),
        angle(0.0f),
        sx(1.0f),
        sy(1.0f),
        kx(0.0f),
        ky(0.0f),
        has_tx(false),
        has_ty(false),
        has_angle(false),
        has_sx(false),
        has_sy(false),
        has_kx(false),
        has_ky(false),
        has_op(false),
        object_type(kObjectTypeBoth),
        has_text_content(false) {}
};

bool TryParseFloat(const String& value, float& out_value) {
  char* end_ptr = NULL;
  out_value = strtof((const char*)value, &end_ptr);
  return end_ptr != NULL && *end_ptr == '\0';
}

bool ParseObjectType(const String& value, ObjectTypeFilter& out) {
  if (value.Equal("text")) { out = kObjectTypeText; return true; }
  if (value.Equal("image")) { out = kObjectTypeImage; return true; }
  if (value.Equal("both")) { out = kObjectTypeBoth; return true; }
  return false;
}

bool ParseTransformOp(const String& value, TransformOp& op) {
  if (value.Equal("translate")) {
    op = kTransformTranslate;
    return true;
  }
  if (value.Equal("rotate")) {
    op = kTransformRotate;
    return true;
  }
  if (value.Equal("scale")) {
    op = kTransformScale;
    return true;
  }
  if (value.Equal("shear")) {
    op = kTransformShear;
    return true;
  }
  return false;
}

bool ValidateTransformCommand(const TransformCommand& cmd) {
  if (!cmd.has_op) {
    return false;
  }

  if (cmd.op == kTransformTranslate) {
    return cmd.has_tx && cmd.has_ty &&
           !cmd.has_angle && !cmd.has_sx && !cmd.has_sy && !cmd.has_kx && !cmd.has_ky;
  }

  if (cmd.op == kTransformRotate) {
    return cmd.has_angle &&
           !cmd.has_tx && !cmd.has_ty && !cmd.has_sx && !cmd.has_sy && !cmd.has_kx && !cmd.has_ky;
  }

  if (cmd.op == kTransformScale) {
    return cmd.has_sx && cmd.has_sy &&
           !cmd.has_tx && !cmd.has_ty && !cmd.has_angle && !cmd.has_kx && !cmd.has_ky;
  }

  if (cmd.op == kTransformShear) {
    return cmd.has_kx && cmd.has_ky &&
           !cmd.has_tx && !cmd.has_ty && !cmd.has_angle && !cmd.has_sx && !cmd.has_sy;
  }

  return false;
}

void PrintUsage() {
  cout << "Usage:" << endl
      << "matrix <input pdf path> <output pdf path> --op <translate|rotate|scale|shear> [params]" << endl
      << endl
      << "input pdf path: The input pdf path." << endl
      << "output pdf path: The output pdf path." << endl
      << "translate params: --tx <float> --ty <float>" << endl
      << "rotate params: --angle <float-radians>" << endl
      << "scale params: --sx <float> --sy <float>" << endl
      << "shear params: --kx <float> --ky <float>" << endl
      << endl
      << "Example:" << endl
      << "matrix input.pdf output.pdf --op rotate --angle 1.57" << endl
      << endl
      << "Object filter options:" << endl
      << "--object-type <text|image|both>  Type of object to transform (default: both)." << endl
      << "--text-content <string>          Only transform text objects matching this content." << endl;
}

bool AnalysisParameter(int argc, char* argv[], WString& input_file, WString& output_file, TransformCommand& cmd) {
  if (argc < 7 || ((argc - 3) % 2 != 0)) {
    return false;
  }

  input_file = WString::FromUTF8(String(argv[1]));
  output_file = WString::FromUTF8(String(argv[2]));

  for (int i = 3; i < argc; i += 2) {
    String key = String(argv[i]);
    String value = String(argv[i + 1]);

    if (key.Equal("--op")) {
      if (!ParseTransformOp(value, cmd.op)) {
        return false;
      }
      cmd.has_op = true;
      continue;
    }

    if (key.Equal("--object-type")) {
      if (!ParseObjectType(value, cmd.object_type)) {
        return false;
      }
      continue;
    }

    if (key.Equal("--text-content")) {
      cmd.text_content = WString::FromUTF8(value);
      cmd.has_text_content = true;
      continue;
    }

    float numeric_value = 0.0f;
    if (!TryParseFloat(value, numeric_value)) {
      return false;
    }

    if (key.Equal("--tx")) {
      cmd.tx = numeric_value;
      cmd.has_tx = true;
    } else if (key.Equal("--ty")) {
      cmd.ty = numeric_value;
      cmd.has_ty = true;
    } else if (key.Equal("--angle")) {
      cmd.angle = numeric_value;
      cmd.has_angle = true;
    } else if (key.Equal("--sx")) {
      cmd.sx = numeric_value;
      cmd.has_sx = true;
    } else if (key.Equal("--sy")) {
      cmd.sy = numeric_value;
      cmd.has_sy = true;
    } else if (key.Equal("--kx")) {
      cmd.kx = numeric_value;
      cmd.has_kx = true;
    } else if (key.Equal("--ky")) {
      cmd.ky = numeric_value;
      cmd.has_ky = true;
    } else {
      return false;
    }
  }

  return ValidateTransformCommand(cmd);
}

static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

class SdkLibMgr {
public:
  SdkLibMgr() : isInitialize(false){};
  ErrorCode Initialize() {
    ErrorCode error_code = Library::Initialize(sn, key);
    if (error_code != foxit::e_ErrSuccess) {
      printf("Library Initialize Error: %d\n", error_code);
    } else {
      isInitialize = true;
    }
    return error_code;

  }
  ~SdkLibMgr(){
    if(isInitialize)
      Library::Release();
  }
private:
  bool isInitialize;
};
int main(int argc, char *argv[])
{
  if ((argc > 1 && String(argv[1]).Equal("--help")) || argc < 2) {
    PrintUsage();
    return 0;
  }

  int err_ret = 0;
  WString input_file;
  WString output_file;
  TransformCommand cmd;
  if (!AnalysisParameter(argc, argv, input_file, output_file, cmd)) {
    PrintUsage();
    return 1;
  }

  SdkLibMgr spSdkLibMgr;
  // Initialize library.
  ErrorCode error_code = spSdkLibMgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    PDFDoc doc(input_file);
    ErrorCode error_code = doc.Load();
    if (error_code != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(input_file), error_code);
      return 1;
    }
    // Get and parse page.
    PDFPage page = doc.GetPage(0);
    page.StartParse(foxit::pdf::PDFPage::e_ParsePageNormal, NULL, false);
    POSITION pos = page.GetFirstGraphicsObjectPosition();
    int flag = 0;
    int stop_mask = 0;
    if (cmd.object_type == kObjectTypeText || cmd.object_type == kObjectTypeBoth) stop_mask |= 2;
    if (cmd.object_type == kObjectTypeImage || cmd.object_type == kObjectTypeBoth) stop_mask |= 4;
    while(pos) {
      if((flag & stop_mask) == stop_mask)
        break;
      graphics::GraphicsObject* obj = page.GetGraphicsObject(pos);
      pos = page.GetNextGraphicsObjectPosition(pos);
      graphics::GraphicsObject::Type type = obj->GetType();
      // Get one TextObject or one ImageObject.
      if(type == graphics::GraphicsObject::e_TypeText && (cmd.object_type == kObjectTypeText || cmd.object_type == kObjectTypeBoth) && !(flag & 2)) {
        if (cmd.has_text_content) {
          graphics::TextObject* text_obj = (graphics::TextObject*)obj;
          WString text = text_obj->GetText();
          if (text.Compare(cmd.text_content) != 0)
            continue;
        }
        flag |= 2;
      } else if(type == graphics::GraphicsObject::e_TypeImage && (cmd.object_type == kObjectTypeImage || cmd.object_type == kObjectTypeBoth) && !(flag & 4)) {
        flag |= 4;
      }else{
        continue;
      }

      // Apply one selected transform operation to a cloned object.
      graphics::GraphicsObject* clone_obj = obj->Clone();
      Matrix matrix = clone_obj->GetMatrix();
      if (cmd.op == kTransformTranslate) {
        matrix.Translate(cmd.tx, cmd.ty);
      } else if (cmd.op == kTransformRotate) {
        matrix.Rotate(cmd.angle);
      } else if (cmd.op == kTransformScale) {
        matrix.Scale(cmd.sx, cmd.sy);
      } else if (cmd.op == kTransformShear) {
        matrix.Shear(cmd.kx, cmd.ky);
      }
      clone_obj->SetMatrix(matrix);
      page.InsertGraphicsObject(NULL, clone_obj);
    }
    // Generate the page content
    page.GenerateContent();

    // Save the pdf document
    doc.SaveAs(output_file);
    cout << "Matrix demo." << endl;

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