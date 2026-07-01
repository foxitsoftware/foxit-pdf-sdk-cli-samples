// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to add form field and
// get form field information.

// Include Foxit SDK header files.
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/interform/fs_pdfform.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace annots;
using namespace actions;
using namespace interform;

struct RectOptions {
  float x;
  float y;
  float w;
  float h;
  bool customized;

  RectOptions() : x(0), y(0), w(100), h(30), customized(false) {}
  RectOptions(float in_x, float in_y, float in_w, float in_h)
      : x(in_x), y(in_y), w(in_w), h(in_h), customized(false) {}
};

struct PushButtonOptions {
  RectOptions rect;
  WString name;
  WString caption;
  String submit_url;
  bool customized;

  PushButtonOptions()
      : rect(50.0f, 750.0f, 100.0f, 30.0f),
        name(L"Push Button Submit"),
        caption(L"Submit"),
        submit_url("http://www.foxitsoftware.com"),
        customized(false) {}
};

struct RadioButtonOptions {
  RectOptions rect;
  WString name;
  WString export_yes;
  WString export_no;
  float second_gap;
  int checked_index;
  bool customized;

  RadioButtonOptions()
      : rect(50.0f, 700.0f, 40.0f, 40.0f),
        name(L"Radio Button0"),
        export_yes(L"YES"),
        export_no(L"NO"),
        second_gap(50.0f),
        checked_index(0),
        customized(false) {}
};

struct CheckBoxOptions {
  RectOptions rect;
  WString name;
  bool checked;
  bool customized;

  CheckBoxOptions()
      : rect(50.0f, 650.0f, 40.0f, 40.0f),
        name(L"Check Box0"),
        checked(true),
        customized(false) {}
};

struct TextFieldOptions {
  RectOptions rect;
  WString name;
  WString value;
  String flags_raw;
  int max_length;
  bool customized;

  TextFieldOptions()
      : rect(50.0f, 600.0f, 40.0f, 40.0f),
        name(L"Text Field0"),
        value(L"3"),
        flags_raw("none"),
        max_length(0),
        customized(false) {}
};

struct ChoiceFieldOptions {
  RectOptions rect;
  WString name;
  String options_raw;
  WString selected;
  bool customized;

  ChoiceFieldOptions()
      : rect(50.0f, 450.0f, 300.0f, 50.0f),
        name(L"List Box0"),
        options_raw("Foxit SDK,Foxit Reader,Foxit Phantom"),
        selected(L"Foxit SDK"),
        customized(false) {}
};

struct CliOptions {
  WString input_file;
  WString output_file;
  String field_types_raw;
  bool show_help;

  PushButtonOptions pushbutton;
  RadioButtonOptions radiobutton;
  CheckBoxOptions checkbox;
  TextFieldOptions textfield;
  ChoiceFieldOptions listbox;
  ChoiceFieldOptions combobox;

  CliOptions()
      : show_help(false) {
    combobox.rect = RectOptions(50.0f, 350.0f, 300.0f, 50.0f);
    combobox.name = L"Combo Box0";
  }
};

void PrintUsage() {
  cout << "Usage:" << endl
       << "demo_form -i <input.pdf> -o <output.pdf> --field-types <types> [options]" << endl
       << endl
       << "Required options:" << endl
       << "  -i, --input              Input PDF path" << endl
       << "  -o, --output             Output PDF path" << endl
       << "  --field-types            Comma-separated field types: pushbutton,radiobutton,checkbox,textfield,listbox,combobox" << endl
       << endl
       << "Per-type rect options:" << endl
       << "  --<type>-x --<type>-y --<type>-w --<type>-h" << endl
       << endl
       << "Push button:" << endl
       << "  --pushbutton-name --pushbutton-caption --pushbutton-url" << endl
       << "Radio button:" << endl
       << "  --radiobutton-name --radiobutton-yes --radiobutton-no --radiobutton-gap --radiobutton-checked (first|second|none)" << endl
       << "Checkbox:" << endl
       << "  --checkbox-name --checkbox-checked (true|false)" << endl
       << "Text field:" << endl
       << "  --textfield-name --textfield-value --textfield-flags (comb,multiline,password,none) --textfield-max-length" << endl
       << "List box / Combo box:" << endl
       << "  --listbox-name --listbox-options --listbox-selected" << endl
       << "  --combobox-name --combobox-options --combobox-selected" << endl
       << endl
       << "Other:" << endl
       << "  --help                   Show this help message" << endl;
}

std::string ToStdString(const String& value) {
  return std::string((const char*)value);
}

std::string ToLowerAscii(const std::string& value) {
  std::string lower = value;
  std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return lower;
}

std::string TrimAscii(const std::string& value) {
  size_t begin = 0;
  while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin]))) ++begin;
  size_t end = value.size();
  while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) --end;
  return value.substr(begin, end - begin);
}

bool ParseFloatValue(const String& value, float& result) {
  std::string text = ToStdString(value);
  char* end_ptr = NULL;
  float parsed = strtof(text.c_str(), &end_ptr);
  if (end_ptr == text.c_str() || *end_ptr != '\0') return false;
  result = parsed;
  return true;
}

bool ParseIntValue(const String& value, int& result) {
  std::string text = ToStdString(value);
  char* end_ptr = NULL;
  long parsed = strtol(text.c_str(), &end_ptr, 10);
  if (end_ptr == text.c_str() || *end_ptr != '\0') return false;
  result = static_cast<int>(parsed);
  return true;
}

bool ParseBoolValue(const String& value, bool& result) {
  std::string text = ToLowerAscii(ToStdString(value));
  if (text == "true" || text == "1" || text == "yes") {
    result = true;
    return true;
  }
  if (text == "false" || text == "0" || text == "no") {
    result = false;
    return true;
  }
  return false;
}

bool SetRectOption(const std::string& key, const String& value, const std::string& prefix,
                   RectOptions& rect, std::string& error_message) {
  float parsed = 0.0f;
  if (!ParseFloatValue(value, parsed)) {
    error_message = "Invalid float value for " + key;
    return false;
  }

  if (key == "--" + prefix + "-x") rect.x = parsed;
  else if (key == "--" + prefix + "-y") rect.y = parsed;
  else if (key == "--" + prefix + "-w") rect.w = parsed;
  else if (key == "--" + prefix + "-h") rect.h = parsed;
  else return false;

  rect.customized = true;
  return true;
}

bool FileExists(const WString& path) {
  FILE* file = NULL;
#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file, String::FromUnicode(path), "rb");
#else
  file = fopen(String::FromUnicode(path), "rb");
#endif
  if (file == NULL) return false;
  fclose(file);
  return true;
}

WString ParentDirectory(const WString& path) {
  std::wstring wpath = std::wstring((FX_LPCWSTR)path, path.GetLength());
  size_t pos = wpath.find_last_of(L'/');
  size_t pos2 = wpath.find_last_of(L'\\');
  if (pos == (size_t)-1 || (pos2 != (size_t)-1 && pos2 > pos)) pos = pos2;
  if (pos == (size_t)-1) return L"";
  std::wstring wpathsub = wpath.substr(0, pos + 1);
  return foxit::WString(wpathsub.c_str(), wpathsub.length());
}

void EnsureDirectoryExists(const WString& directory) {
  if (directory.IsEmpty()) return;
#if defined(_WIN32) || defined(_WIN64)
  _mkdir(String::FromUnicode(directory));
#else
  mkdir(String::FromUnicode(directory), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

std::vector<std::string> ParseCsvTokens(const std::string& text) {
  std::vector<std::string> tokens;
  size_t start = 0;
  while (start <= text.size()) {
    size_t comma = text.find(',', start);
    std::string token = (comma == std::string::npos) ? text.substr(start) : text.substr(start, comma - start);
    token = TrimAscii(token);
    if (!token.empty()) tokens.push_back(token);
    if (comma == std::string::npos) break;
    start = comma + 1;
  }
  return tokens;
}

bool ParseFieldTypes(const String& raw, std::vector<std::string>& field_types, std::string& error_message) {
  std::vector<std::string> tokens = ParseCsvTokens(ToLowerAscii(ToStdString(raw)));
  std::set<std::string> used;
  for (size_t i = 0; i < tokens.size(); ++i) {
    const std::string& token = tokens[i];
    if (token != "pushbutton" && token != "radiobutton" && token != "checkbox" &&
        token != "textfield" && token != "listbox" && token != "combobox") {
      error_message = "Invalid field type: " + token;
      return false;
    }
    if (used.find(token) == used.end()) {
      used.insert(token);
      field_types.push_back(token);
    }
  }

  if (field_types.empty()) {
    error_message = "--field-types is empty. Please provide at least one type.";
    return false;
  }
  return true;
}

bool ParseArgs(int argc, char* argv[], CliOptions& options, std::string& error_message) {
  for (int i = 1; i < argc; ++i) {
    String key_obj = String(argv[i]);
    std::string key = ToLowerAscii(ToStdString(key_obj));

    if (key == "--help") {
      options.show_help = true;
      return true;
    }

    if (i + 1 >= argc) {
      error_message = "Missing value for argument: " + key;
      return false;
    }

    String value = String(argv[++i]);
    if (key == "-i" || key == "--input") {
      options.input_file = WString::FromUTF8(value);
      continue;
    }
    if (key == "-o" || key == "--output") {
      options.output_file = WString::FromUTF8(value);
      continue;
    }
    if (key == "--field-types") {
      options.field_types_raw = value;
      continue;
    }

    if (SetRectOption(key, value, "pushbutton", options.pushbutton.rect, error_message)) {
      options.pushbutton.customized = true;
      continue;
    }
    if (SetRectOption(key, value, "radiobutton", options.radiobutton.rect, error_message)) {
      options.radiobutton.customized = true;
      continue;
    }
    if (SetRectOption(key, value, "checkbox", options.checkbox.rect, error_message)) {
      options.checkbox.customized = true;
      continue;
    }
    if (SetRectOption(key, value, "textfield", options.textfield.rect, error_message)) {
      options.textfield.customized = true;
      continue;
    }
    if (SetRectOption(key, value, "listbox", options.listbox.rect, error_message)) {
      options.listbox.customized = true;
      continue;
    }
    if (SetRectOption(key, value, "combobox", options.combobox.rect, error_message)) {
      options.combobox.customized = true;
      continue;
    }

    if (key == "--pushbutton-name") {
      options.pushbutton.name = WString::FromUTF8(value);
      options.pushbutton.customized = true;
      continue;
    }
    if (key == "--pushbutton-caption") {
      options.pushbutton.caption = WString::FromUTF8(value);
      options.pushbutton.customized = true;
      continue;
    }
    if (key == "--pushbutton-url") {
      options.pushbutton.submit_url = value;
      options.pushbutton.customized = true;
      continue;
    }

    if (key == "--radiobutton-name") {
      options.radiobutton.name = WString::FromUTF8(value);
      options.radiobutton.customized = true;
      continue;
    }
    if (key == "--radiobutton-yes") {
      options.radiobutton.export_yes = WString::FromUTF8(value);
      options.radiobutton.customized = true;
      continue;
    }
    if (key == "--radiobutton-no") {
      options.radiobutton.export_no = WString::FromUTF8(value);
      options.radiobutton.customized = true;
      continue;
    }
    if (key == "--radiobutton-gap") {
      float parsed = 0.0f;
      if (!ParseFloatValue(value, parsed)) {
        error_message = "Invalid float value for --radiobutton-gap";
        return false;
      }
      options.radiobutton.second_gap = parsed;
      options.radiobutton.customized = true;
      continue;
    }
    if (key == "--radiobutton-checked") {
      std::string checked = ToLowerAscii(ToStdString(value));
      if (checked == "first") options.radiobutton.checked_index = 0;
      else if (checked == "second") options.radiobutton.checked_index = 1;
      else if (checked == "none") options.radiobutton.checked_index = -1;
      else {
        error_message = "Invalid value for --radiobutton-checked, expected first|second|none";
        return false;
      }
      options.radiobutton.customized = true;
      continue;
    }

    if (key == "--checkbox-name") {
      options.checkbox.name = WString::FromUTF8(value);
      options.checkbox.customized = true;
      continue;
    }
    if (key == "--checkbox-checked") {
      bool parsed = false;
      if (!ParseBoolValue(value, parsed)) {
        error_message = "Invalid bool value for --checkbox-checked";
        return false;
      }
      options.checkbox.checked = parsed;
      options.checkbox.customized = true;
      continue;
    }

    if (key == "--textfield-name") {
      options.textfield.name = WString::FromUTF8(value);
      options.textfield.customized = true;
      continue;
    }
    if (key == "--textfield-value") {
      options.textfield.value = WString::FromUTF8(value);
      options.textfield.customized = true;
      continue;
    }
    if (key == "--textfield-flags") {
      options.textfield.flags_raw = value;
      options.textfield.customized = true;
      continue;
    }
    if (key == "--textfield-max-length") {
      int parsed = 0;
      if (!ParseIntValue(value, parsed)) {
        error_message = "Invalid int value for --textfield-max-length";
        return false;
      }
      options.textfield.max_length = parsed;
      options.textfield.customized = true;
      continue;
    }

    if (key == "--listbox-name") {
      options.listbox.name = WString::FromUTF8(value);
      options.listbox.customized = true;
      continue;
    }
    if (key == "--listbox-options") {
      options.listbox.options_raw = value;
      options.listbox.customized = true;
      continue;
    }
    if (key == "--listbox-selected") {
      options.listbox.selected = WString::FromUTF8(value);
      options.listbox.customized = true;
      continue;
    }

    if (key == "--combobox-name") {
      options.combobox.name = WString::FromUTF8(value);
      options.combobox.customized = true;
      continue;
    }
    if (key == "--combobox-options") {
      options.combobox.options_raw = value;
      options.combobox.customized = true;
      continue;
    }
    if (key == "--combobox-selected") {
      options.combobox.selected = WString::FromUTF8(value);
      options.combobox.customized = true;
      continue;
    }

    error_message = "Unknown argument: " + key;
    return false;
  }

  return true;
}

bool ValidateArgs(const CliOptions& options, const std::set<std::string>& selected_types, std::string& error_message) {
  if (options.input_file.IsEmpty()) {
    error_message = "Missing required argument: --input";
    return false;
  }
  if (options.output_file.IsEmpty()) {
    error_message = "Missing required argument: --output";
    return false;
  }
  if (options.field_types_raw.IsEmpty()) {
    error_message = "Missing required argument: --field-types";
    return false;
  }

  if (options.pushbutton.customized && selected_types.find("pushbutton") == selected_types.end()) {
    error_message = "pushbutton parameters provided but pushbutton is not in --field-types";
    return false;
  }
  if (options.radiobutton.customized && selected_types.find("radiobutton") == selected_types.end()) {
    error_message = "radiobutton parameters provided but radiobutton is not in --field-types";
    return false;
  }
  if (options.checkbox.customized && selected_types.find("checkbox") == selected_types.end()) {
    error_message = "checkbox parameters provided but checkbox is not in --field-types";
    return false;
  }
  if (options.textfield.customized && selected_types.find("textfield") == selected_types.end()) {
    error_message = "textfield parameters provided but textfield is not in --field-types";
    return false;
  }
  if (options.listbox.customized && selected_types.find("listbox") == selected_types.end()) {
    error_message = "listbox parameters provided but listbox is not in --field-types";
    return false;
  }
  if (options.combobox.customized && selected_types.find("combobox") == selected_types.end()) {
    error_message = "combobox parameters provided but combobox is not in --field-types";
    return false;
  }

  if (options.textfield.max_length < 0) {
    error_message = "--textfield-max-length must be >= 0";
    return false;
  }

  if (ParseCsvTokens(ToStdString(options.listbox.options_raw)).empty()) {
    error_message = "--listbox-options is empty";
    return false;
  }
  if (ParseCsvTokens(ToStdString(options.combobox.options_raw)).empty()) {
    error_message = "--combobox-options is empty";
    return false;
  }

  return true;
}

RectF ToRectF(const RectOptions& rect) {
  return RectF(rect.x, rect.y, rect.x + rect.w, rect.y + rect.h);
}

uint32 ParseTextFieldFlags(const String& flags_raw, std::string& error_message) {
  std::vector<std::string> tokens = ParseCsvTokens(ToLowerAscii(ToStdString(flags_raw)));
  if (tokens.empty()) tokens.push_back("none");

  uint32 flags = 0;
  for (size_t i = 0; i < tokens.size(); ++i) {
    const std::string& token = tokens[i];
    if (token == "none") {
      continue;
    } else if (token == "comb") {
      flags |= Field::e_FlagTextComb;
    } else if (token == "multiline") {
      flags |= Field::e_FlagTextMultiline;
    } else if (token == "password") {
      flags |= Field::e_FlagTextPassword;
    } else {
      error_message = "Invalid textfield flag: " + token;
      return 0;
    }
  }
  return flags;
}

ChoiceOptionArray BuildChoiceOptions(const String& options_raw) {
  ChoiceOptionArray options;
  std::vector<std::string> tokens = ParseCsvTokens(ToStdString(options_raw));
  for (size_t i = 0; i < tokens.size(); ++i) {
    WString text = WString::FromUTF8(String(tokens[i].c_str()));
    options.Add(interform::ChoiceOption(text, text, true, true));
  }
  return options;
}

void AddPushButton(PDFPage& page, interform::Form& form, const PushButtonOptions& options) {
  Control control = form.AddControl(page, options.name, Field::e_TypePushButton, ToRectF(options.rect));

  foxit::pdf::DefaultAppearance default_ap;
  default_ap.flags = DefaultAppearance::e_FlagFont | DefaultAppearance::e_FlagFontSize | DefaultAppearance::e_FlagTextColor;
  default_ap.font = Font(Font::e_StdIDHelveticaB);
  default_ap.text_size = 12.0f;
  default_ap.text_color = 0x000000;
  form.SetDefaultAppearance(default_ap);

  Widget widget = control.GetWidget();
  widget.SetHighlightingMode(foxit::pdf::annots::Annot::e_HighlightingPush);
  widget.SetMKBorderColor(0xFF0000);
  widget.SetMKBackgroundColor(0xF0F0F0);
  widget.SetMKNormalCaption(options.caption);
  widget.ResetAppearanceStream();

  if (!options.submit_url.IsEmpty()) {
    actions::SubmitFormAction submit_action = (actions::SubmitFormAction)Action::Create(form.GetDocument(), Action::e_TypeSubmitForm);
    int count = form.GetFieldCount(NULL);
    WStringArray name_array;
    for (int i = 0; i < count; ++i) {
      name_array.Add(form.GetField(i, NULL).GetName());
    }
    submit_action.SetFieldNames(name_array);
    submit_action.SetURL(options.submit_url);
    widget.SetAction(submit_action);
  }
  cout << "Add pushbutton field." << endl;
}

void AddRadioButton(PDFPage& page, interform::Form& form, const RadioButtonOptions& options) {
  RectF rect0 = ToRectF(options.rect);
  RectF rect1(options.rect.x + options.second_gap,
              options.rect.y,
              options.rect.x + options.second_gap + options.rect.w,
              options.rect.y + options.rect.h);
  Control control0 = form.AddControl(page, options.name, Field::e_TypeRadioButton, rect0);
  Control control1 = form.AddControl(page, options.name, Field::e_TypeRadioButton, rect1);

  control0.SetExportValue(options.export_yes);
  control1.SetExportValue(options.export_no);
  control0.SetChecked(options.checked_index == 0);
  control1.SetChecked(options.checked_index == 1);

  control0.GetWidget().ResetAppearanceStream();
  control1.GetWidget().ResetAppearanceStream();
  cout << "Add radiobutton field." << endl;
}

void AddCheckBox(PDFPage& page, interform::Form& form, const CheckBoxOptions& options) {
  Control control = form.AddControl(page, options.name, Field::e_TypeCheckBox, ToRectF(options.rect));
  control.SetChecked(options.checked);

  Widget widget = control.GetWidget();
  widget.SetMKBorderColor(0x000000);
  widget.SetMKBackgroundColor(0xFFFFFF);
  widget.ResetAppearanceStream();
  cout << "Add checkbox field." << endl;
}

bool AddTextField(PDFPage& page, interform::Form& form, const TextFieldOptions& options, std::string& error_message) {
  Control control = form.AddControl(page, options.name, Field::e_TypeTextField, ToRectF(options.rect));
  Field field = control.GetField();
  field.SetValue(options.value);

  uint32 flags = ParseTextFieldFlags(options.flags_raw, error_message);
  if (!error_message.empty()) return false;
  if (flags != 0) field.SetFlags(flags);
  if (options.max_length > 0) field.SetMaxLength(options.max_length);

  control.GetWidget().ResetAppearanceStream();
  cout << "Add textfield field." << endl;
  return true;
}

void AddListBox(PDFPage& page, interform::Form& form, const ChoiceFieldOptions& options) {
  Control control = form.AddControl(page, options.name, Field::e_TypeListBox, ToRectF(options.rect));
  Field field = control.GetField();
  field.SetOptions(BuildChoiceOptions(options.options_raw));
  if (!options.selected.IsEmpty()) field.SetValue(options.selected);

  Widget widget = control.GetWidget();
  widget.SetMKBorderColor(0x000000);
  widget.SetMKBackgroundColor(0xFFFFFF);
  widget.ResetAppearanceStream();
  cout << "Add listbox field." << endl;
}

void AddComboBox(PDFPage& page, interform::Form& form, const ChoiceFieldOptions& options) {
  Control control = form.AddControl(page, options.name, Field::e_TypeComboBox, ToRectF(options.rect));
  Field field = control.GetField();
  field.SetOptions(BuildChoiceOptions(options.options_raw));
  if (!options.selected.IsEmpty()) field.SetValue(options.selected);

  Widget widget = control.GetWidget();
  widget.SetMKBorderColor(0x000000);
  widget.SetMKBackgroundColor(0xFFFFFF);
  widget.ResetAppearanceStream();
  cout << "Add combobox field." << endl;
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

int main(int argc, char *argv[])
{
  CliOptions options;
  std::string error_message;
  if (!ParseArgs(argc, argv, options, error_message)) {
    cout << error_message << endl;
    PrintUsage();
    return 1;
  }
  if (options.show_help) {
    PrintUsage();
    return 0;
  }

  std::vector<std::string> field_types;
  if (!ParseFieldTypes(options.field_types_raw, field_types, error_message)) {
    cout << error_message << endl;
    PrintUsage();
    return 1;
  }
  std::set<std::string> selected_types(field_types.begin(), field_types.end());
  if (!ValidateArgs(options, selected_types, error_message)) {
    cout << error_message << endl;
    PrintUsage();
    return 1;
  }

  if (!FileExists(options.input_file)) {
    cout << "Input file does not exist: " << String::FromUnicode(options.input_file) << endl;
    return 1;
  }
  if (FileExists(options.output_file)) {
    cout << "Output file already exists, refusing to overwrite: " << String::FromUnicode(options.output_file) << endl;
    return 1;
  }

  EnsureDirectoryExists(ParentDirectory(options.output_file));

  int err_ret = 0;
  SdkLibMgr sdk_lib_mgr;
  // Initialize library
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  try {
    PDFDoc doc(options.input_file);
    ErrorCode load_error = doc.Load();
    if (load_error != foxit::e_ErrSuccess) {
      printf("The Doc [%s] Error: %d\n", (const char*)String::FromUnicode(options.input_file), load_error);
      return 1;
    }

    interform::Form form(doc);
    PDFPage page = doc.GetPage(0);
    if (page.IsEmpty()) {
      page = doc.InsertPage(0);
    } else {
      page.StartParse();
    }

    for (size_t i = 0; i < field_types.size(); ++i) {
      const std::string& type = field_types[i];
      if (type == "pushbutton") {
        AddPushButton(page, form, options.pushbutton);
      } else if (type == "radiobutton") {
        AddRadioButton(page, form, options.radiobutton);
      } else if (type == "checkbox") {
        AddCheckBox(page, form, options.checkbox);
      } else if (type == "textfield") {
        std::string textfield_error;
        if (!AddTextField(page, form, options.textfield, textfield_error)) {
          cout << textfield_error << endl;
          return 1;
        }
      } else if (type == "listbox") {
        AddListBox(page, form, options.listbox);
      } else if (type == "combobox") {
        AddComboBox(page, form, options.combobox);
      }
    }

    if (!doc.SaveAs(options.output_file, PDFDoc::e_SaveFlagNoOriginal)) {
      cout << "Save failed: " << String::FromUnicode(options.output_file) << endl;
      return 1;
    }
    cout << "form Finish : All selected form fields generated successfully" << endl;
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
