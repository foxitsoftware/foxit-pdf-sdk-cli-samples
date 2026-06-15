
// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to add various annotations to PDF document.

// Include Foxit SDK header files.
#include <time.h>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <cstdlib>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

#include "../../../include/common/fs_common.h"

#include "../../../include/pdf/actions/fs_action.h"
#include "../../../include/pdf/fs_signature.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/fs_actioncallback.h"
#include "../../../include/pdf/annots/fs_annot.h"
#include "../../../include/pdf/objects/fs_pdfobject.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/fs_psi.h"
#include "../../../include/pdf/fs_filespec.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace annots;
using foxit::pdf::annots::Line;

static const char* sn = "";
static const char* key = "";

static WString input_path;  // directory of the input file, set at runtime

// CLI parameter globals
static std::string g_operation = "add";               // add, delete, or edit
static std::set<std::string> g_types;                    // empty = all
static std::string g_input_path = "annotation_input.pdf";
static std::string g_output_path = "annotation_output.pdf";
static int g_page_index = 0;
static std::string g_author_str = "Foxit SDK";
static int g_flags = 4;  // annotation flags; 4 = Print
static bool g_has_content = false;
static std::string g_content_str;
static bool g_has_subject = false;
static std::string g_subject_str;
static bool g_has_rect = false;
static RectF g_rect;
static bool g_has_inner_rect = false;
static RectF g_inner_rect;
static bool g_has_vertexes = false;
static foxit::PointFArray g_vertexes;
static bool g_has_quad_points = false;
static foxit::PointFArray g_quad_points_raw;
static bool g_has_ink_points = false;
static foxit::PointFArray g_ink_points;
static bool g_has_color = false;
static int g_color = 0;
static bool g_has_fill_color = false;
static int g_fill_color = 0;
static bool g_has_border = false;
static float g_border_width = 1.0f;
static std::string g_border_style = "solid";
static float g_border_intensity = 0.0f;
static bool g_has_line_start = false;
static PointF g_line_start(20.f, 650.f);
static bool g_has_line_end = false;
static PointF g_line_end(100.f, 740.f);
static std::string g_line_intent = "LineArrow";
static std::string g_polygon_style = "cloud";
static bool g_has_freetext_intent = false;
static std::string g_freetext_intent = "typewriter";
static bool g_richtext = false;
static std::string g_rt_font;
static bool g_has_rt_font_size = false;
static float g_rt_font_size = 10.0f;
static bool g_has_rt_text_color = false;
static int g_rt_text_color = 0xFF0000;
static bool g_has_da_font_size = false;
static float g_da_font_size = 12.0f;
static bool g_has_da_text_color = false;
static int g_da_text_color = 0x000000;
static bool g_has_callout_points = false;
static foxit::PointFArray g_callout_points;
static bool g_dynamic_stamp = false;
static bool g_popup = true;
static bool g_popup_open = false;
static bool g_has_popup_rect = false;
static RectF g_popup_rect;
static bool g_has_popup_color = false;
static int g_popup_color = 0x00FF00;
static std::string g_icon_name;
static std::string g_stamps_dir;
static std::string g_attach_file;
static std::string g_attach_name;
static std::string g_attach_desc;
static std::string g_link_highlight = "push";
static std::string g_video_file;
static std::string g_screen_image;
static std::string g_password;

// Returns true when the given annotation type should be added.
static bool ShouldAddType(const char* type) {
  return g_types.empty() || g_types.count("all") > 0 || g_types.count(type) > 0;
}

// Returns the user-supplied rect if set, otherwise the default.
static RectF GetRect(RectF default_rect) {
  return g_has_rect ? g_rect : default_rect;
}

// Build BorderInfo from CLI parameters.
static BorderInfo GetBorderInfo() {
  BorderInfo bi;
  bi.width = g_border_width;
  bi.cloud_intensity = g_border_intensity;
  if (g_border_style == "dashed") bi.style = BorderInfo::e_Dashed;
  else if (g_border_style == "underline") bi.style = BorderInfo::e_UnderLine;
  else if (g_border_style == "beveled") bi.style = BorderInfo::e_Beveled;
  else if (g_border_style == "inset") bi.style = BorderInfo::e_Inset;
  else if (g_border_style == "cloudy") bi.style = BorderInfo::e_Cloudy;
  else bi.style = BorderInfo::e_Solid;
  return bi;
}

static bool IsValidType(const char* t) {
  static const char* kValid[] = {
    "all","line","circle","square","polygon","polyline",
    "freetext","highlight","underline","squiggly","strikeout",
    "caret","note","ink","link","stamp","screen", "fileattachment", nullptr
  };
  for (int i = 0; kValid[i]; i++)
    if (strcmp(kValid[i], t) == 0) return true;
  return false;
}

static bool ParseHexColor(const char* hex, int* out) {
  if (!hex) return false;
  size_t len = strlen(hex);
  if (len == 0 || len > 6) return false;
  for (size_t i = 0; i < len; i++) {
    char c = hex[i];
    if (!((c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F')))
      return false;
  }
  *out = (int)strtol(hex, nullptr, 16);
  return true;
}

static bool ParseXY(const char* s, float* x, float* y) {
  return sscanf(s, "%f,%f", x, y) == 2;
}

static bool ParseXYZW(const char* s, float* x1, float* y1, float* x2, float* y2) {
  return sscanf(s, "%f,%f,%f,%f", x1, y1, x2, y2) == 4;
}

// Parse semicolon-separated points: "x1,y1;x2,y2;x3,y3;..."
static bool ParseVertexes(const char* s, foxit::PointFArray& out) {
  out.RemoveAll();
  const char* p = s;
  while (*p) {
    float x, y;
    int n = 0;
    if (sscanf(p, "%f,%f%n", &x, &y, &n) < 2 || n == 0) return false;
    out.Add(PointF(x, y));
    p += n;
    if (*p == ';') ++p;
    else if (*p != '\0') return false;
  }
  return out.GetSize() >= 2;
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

DateTime GetLocalDateTime() {
  time_t t = time(NULL);
#if (WINAPI_PARTITION_APP || WINAPI_PARTITION_PC_APP) || \
  (defined(_WIN32) || defined(_WIN64)) && _FX_COMPILER_ != _FX_VC6_
  struct tm _Tm;
  localtime_s(&_Tm, &t);
  struct tm* rime = &_Tm;
  _tzset();
  long time_zone = NULL;
  _get_timezone(&time_zone);
  int timezone_hour = time_zone / 3600 * -1;
  int timezone_minute = (abs(time_zone) % 3600) / 60;
#elif defined(__linux__)
  struct tm* rime = localtime(&t);
  tzset();
  int timezone_hour = __timezone / 3600 * -1;
  int timezone_minute = ((int)abs(__timezone) % 3600) / 60;
#elif defined(__APPLE__)
  struct tm* rime = localtime(&t);
  tzset();
  int timezone_hour = timezone / 3600 * -1;
  int timezone_minute = ((int)abs(timezone) % 3600) / 60;
#endif
  DateTime datetime;
  datetime.year = static_cast<uint16>(rime->tm_year + 1900);
  datetime.month = static_cast<uint16>(rime->tm_mon + 1);
  datetime.day = static_cast<uint16>(rime->tm_mday);
  datetime.hour = static_cast<uint16>(rime->tm_hour);
  datetime.minute = static_cast<uint16>(rime->tm_min);
  datetime.second = static_cast<uint16>(rime->tm_sec);
  datetime.utc_hour_offset = timezone_hour;
  datetime.utc_minute_offset = timezone_minute;

  return datetime;
}

String RandomUID() {
  String uuid;
  const char* c = "0123456789qwertyuiopasdfghjklzxcvbnm";

  for (int n = 0; n < 16; n++) {
    String uuid_temp;
    int b = rand() % 255;
    switch (n) {
    case 6:
      uuid_temp.Format("4%x", b % 15);
      break;
    case 8:
      uuid_temp.Format("%c%x", c[rand() % strlen(c)], b % 15);
      break;
    default:
      uuid_temp.Format("%02x", b);
      break;
    }
    uuid += uuid_temp;

    switch (n) {
    case 3:
    case 5:
    case 7:
    case 9:
      uuid += '-';
      break;
    }
  }
  return uuid;
}
class IconProvider : public IconProviderCallback {
public:
  static IconProvider* Create(string path) {
    return new IconProvider(path);
  }
  virtual void Release() {
    delete this;
  }

  // If one icon provider offers different style icon for one icon name of a kind of annotaion,
  // please use different provider ID or version in order to distinguish different style for Foxit PDF SDK.
  // Otherwise, only the first style icon for the same icon name of same kind of annotation will have effect.
  virtual String GetProviderID() {
    if (use_dynamic_stamp_)
      return "Simple Demo Dynamic IconProvider";
    else
      return "Simple Demo IconProvider";
  }

  virtual String GetProviderVersion() {
    return "1.0.0";
  }

  virtual bool HasIcon(Annot::Type annot_type, const char* icon_name) {
    std::map<string, PDFDoc>::iterator it = pdf_doc_map_.find(icon_name);

    if(it != pdf_doc_map_.end())
    {
      PDFDoc doc = (*it).second;
      return !doc.IsEmpty();
    }
    PDFDoc doc = PDFDoc();
    string path;
    if (doc.GetPageCount()==0) {
      try {
        if (use_dynamic_stamp_) {
          path = file_folder_ + "/DynamicStamps/" + icon_name + ".pdf";
        } else {
          path = file_folder_ + "/StaticStamps/" + icon_name + ".pdf";
        }
        doc = PDFDoc(path.c_str());
        ErrorCode error_code = doc.Load();
        if (foxit::e_ErrSuccess != error_code) {
          doc = PDFDoc();
        } else {
          pdf_doc_map_.insert(std::pair<string, PDFDoc>(path, doc));
        }
      } catch (const Exception& e) {
        cout << e.GetMessage() << endl;
      }
    }
    return !doc.IsEmpty();
  }

  virtual bool CanChangeColor(Annot::Type annot_type, const char* icon_name) {
    return false;
  }

  virtual PDFPage GetIcon(Annot::Type annot_type, const char* icon_name, foxit::ARGB color, foxit::pdf::objects::PDFDictionary* annot_dict) {
    string path;
    if (use_dynamic_stamp_) {
      path = file_folder_ + "/DynamicStamps/" + icon_name + ".pdf";
    } else {
      path = file_folder_ + "/StaticStamps/" + icon_name + ".pdf";
    }
    std::map<string, PDFDoc>::iterator it = pdf_doc_map_.find(path);

    PDFDoc doc = ((it != pdf_doc_map_.end()) ? (*it).second : PDFDoc());
    if (doc.IsEmpty() || doc.GetPageCount() == 0)
      return PDFPage();
    return doc.GetPage(0);
  }

  virtual bool GetShadingColor(Annot::Type annot_type, const char* icon_name, foxit::RGB referenced_color,
    int shading_index, foxit::pdf::annots::ShadingColor& shading_color) {
      return false;
  }

  virtual float GetDisplayWidth(Annot::Type annot_type, const char* icon_name) {
    return 0.0f;
  }

  virtual float GetDisplayHeight(Annot::Type annot_type, const char* icon_name) {
    return 0.0f;
  }

  void SetUseDynamicStamp(bool use_dynamic_stamp) {
    use_dynamic_stamp_ = use_dynamic_stamp;
  }

private:
  IconProvider(string file_folder)
    : file_folder_(file_folder)
    , use_dynamic_stamp_(false)
    {}
  virtual ~IconProvider() {
  }
  std::map<string, PDFDoc> pdf_doc_map_;
  string file_folder_;
  vector<string> incon_names_;
  bool use_dynamic_stamp_;
};

class ActionEvent : public foxit::ActionCallback {
public:
  ActionEvent(const PDFDoc& document) {current_doc_ = document;}
  ActionEvent() {}
  ~ActionEvent() {}
  virtual void Release() {delete this;}
  virtual bool InvalidateRect(const PDFDoc& document, int page_index, const RectF& pdf_rect) {
    return false;
  }
  virtual int GetCurrentPage(const PDFDoc& document) {
    return -1;
  }
  virtual void SetCurrentPage(const PDFDoc& document, int page_index) {}
  virtual void SetCurrentPage(const pdf::PDFDoc& document, const foxit::pdf::Destination& destination){}
  virtual foxit::common::Rotation GetPageRotation(const PDFDoc& document, int page_index) {
    return foxit::common::e_Rotation0;
  }
  virtual bool SetPageRotation(const PDFDoc& document, int page_index, foxit::common::Rotation rotation) {
    return false;
  }
  virtual bool ExecuteNamedAction(const PDFDoc& document, const char* named_action) {
    return false;
  }
  virtual bool SetDocChangeMark(const PDFDoc& document, bool change_mark) {
    return false;
  }
  virtual bool GetDocChangeMark(const PDFDoc& document) {
    return false;
  }
  virtual int GetOpenedDocCount() {
    return -1;
  }
  virtual PDFDoc GetOpenedDoc(int index) {
    return current_doc_;
  }
  PDFDoc GetCurrentDoc() {
    return current_doc_;
  }
  PDFDoc CreateBlankDoc() {
    return PDFDoc();
  }

  PDFDoc OpenDoc(const foxit::WString& file_path, const foxit::WString& password) {
    return PDFDoc(file_path);
  }
  virtual void CloseDoc(const pdf::PDFDoc& document, bool is_prompt_to_save) {}
  virtual bool Beep(int type) {
    return false;
  }
  virtual WString Response(const wchar_t* question, const wchar_t* title, const wchar_t* default_value, const wchar_t* label,
    bool is_password) {
      return L"";
  }
  virtual WString GetFilePath(const PDFDoc& document) {
    return L"";
  }
  virtual bool IsLocalFile(const pdf::PDFDoc& document) { return true; }
  virtual bool Print(const PDFDoc& document, bool is_ui,
    const common::Range& page_set, bool is_silent ,
    bool is_shrunk_to_fit, bool is_printed_as_image,
    bool is_reversed, bool is_to_print_annots) {
      return false;
  }
  virtual bool Print(const pdf::PDFDoc& document, const PrintParams& print_params) { return false; }
  virtual bool SubmitForm(const PDFDoc& document, void* form_data, foxit::uint32 length, const char* url, foxit::common::FileFormatType file_format_type) {
    return false;
  }
  virtual bool LaunchURL(const char* url) {
    return false;
  }
  virtual WString BrowseFile() {
    return L"";
  }
  virtual WString BrowseFile(bool is_open_dialog, const wchar_t* file_format, const wchar_t* file_filter) {
    return L"";
  }
  virtual foxit::ActionCallback::Language GetLanguage() {
    return foxit::ActionCallback::e_LanguageCHS;
  }
  virtual int Alert(const wchar_t* msg, const wchar_t* title, int type, int icon) {
    return 0;
  }
  virtual foxit::IdentityProperties GetIdentityProperties() {
    return foxit::IdentityProperties(L"foxitsoftware", L"simple_demo@foxitsoftware.cn", L"simple demo", L"Simple", L"simple", L"demo", L"developer", L"gsdk");
  }
  virtual bool SetIdentityProperties(const foxit::IdentityProperties& identity_properties) {
    return false;
  }
  virtual WString PopupMenu(const foxit::MenuListArray& menus, bool& is_selected_item) {
    return L"";
  }

  foxit::MenuItemEx PopupMenuEx(const foxit::MenuItemExArray& menus, bool& is_selected_item)
  {
    return foxit::MenuItemEx();
  }

  virtual WString GetAppInfo(foxit::ActionCallback::AppInfoType type) {
    return L"";
  }
  virtual bool MailData(void* data, MailType data_type, bool is_ui, const wchar_t* to,
    const wchar_t* subject, const wchar_t* cc, const wchar_t* bcc, const wchar_t* message) {
      return false;
  }
  virtual JsMailResult MailDoc(const pdf::PDFDoc& document,
    const wchar_t* to_address, const wchar_t* cc_address, const wchar_t* bcc_address,
    const wchar_t* subject, const wchar_t* message, bool is_ui) {
    return JsMailResult::e_JSMailResultFailed;
  }
  virtual uint32 VerifySignature(const pdf::PDFDoc& document, const pdf::Signature& pdf_signature) {
    return pdf::Signature::e_StateUnknown;
  }
  virtual WString GetUntitledBookmarkName() { return L"Untitled"; }

  virtual WStringArray GetPrinterNameList() {
    return WStringArray();
  }

  virtual bool AddToolButton(const ButtonItem& button_item) { return false; }
  virtual bool RemoveToolButtom(const wchar_t* button_name) { return false; }
  virtual MenuListArray GetMenuItemNameList() { return MenuListArray(); }
  virtual bool AddSubMenu(const MenuItemConfig& menu_item_config) { return false; }
  virtual bool AddMenuItem(const MenuItemConfig& menu_item_config, bool is_prepend) { return false; }
  virtual bool ShowDialog(const DialogDescriptionConfig& dlg_config) { return true; }
  virtual bool GetFullScreen() { return false; }
  virtual void SetFullScreen(bool is_full_screen) {}
  virtual void OnFieldValueChanged(const wchar_t* field_name, JSFieldValueChangeType type, const WStringArray &value_before_changed, const WStringArray &value_after_changed) {}
  virtual MediaPlayerCallback* OpenMediaPlayer(const PlayerArgs& player_args) { return NULL; }
  virtual WString GetAttachmentsFilePath(const pdf::PDFDoc& pdf_doc, const wchar_t* name) { return L""; }
  virtual WString GetExtractedEmbeddedFilePath(const pdf::PDFDoc& pdf_doc, const wchar_t* name) { return L""; }
  virtual void UpdateLogicalLabel() {}
  virtual WString GetTemporaryFileName(const pdf::PDFDoc& document, const wchar_t* file_name) { return L""; }
  virtual WString GetTemporaryDirectory() { return L""; }
  virtual void Scroll(const PointF& point) {}
  virtual void SelectPageNthWord(int page_index, int start_offset, int end_offset, bool is_show_selection) {}
  virtual PointF GetMousePosition() { return PointF(); }
  virtual RectF GetPageWindowRect(const pdf::PDFDoc& pdf_doc, int ) { return RectF(); }
  virtual LayoutMode GetLayoutMode() { return LayoutMode::e_LayoutModeContinuous; }
  virtual void SetLayoutMode(LayoutMode layout_mode, bool is_cover_mode) {}
  virtual float GetPageScale() { return 0; }
  virtual void SetPageScale(foxit::pdf::Destination::ZoomMode zoom_mode, const foxit::pdf::Destination& dest) {}
  virtual Destination::ZoomMode GetPageZoomMode() { return Destination::e_ZoomFitBHorz; }
  virtual void Query(const wchar_t* keywords, SearchScope search_scope, const SearchOption& search_option, const wchar_t* di_path) {}
  virtual SearchIndexConfig AddSearchIndex(const wchar_t* di_path, bool is_selected) { return SearchIndexConfig(); }
  virtual bool RemoveSearchIndex(const SearchIndexConfig& search_index_config) { return false; }
  virtual WStringArray GetSignatureAPStyleNameList() { return WStringArray(); }
  virtual SOAPResponseInfo SoapRequest(const SOAPRequestProperties& request_params) {return SOAPResponseInfo();}
  virtual void EnablePageLoop(bool is_loop) {}
  virtual bool IsPageLoop() {return true;}
  virtual void SetDefaultPageTransitionMode(const wchar_t* trans_type, const wchar_t* trans_di) {}
  virtual bool IsCurrentDocOpenedInBrowser() { return false; }
  virtual void PostMessageToHtml(WStringArray message) {}
  virtual void NotifyBeginDoJob(const pdf::PDFDoc& document, JavascriptModifyItemInfo::JavascriptEventType event_type) {}
  virtual void NotifyAfterDataChange(const pdf::PDFDoc& document, JavascriptModifyItemInfo modify_item_info) {}
  virtual void NotifyEndDoJob(const pdf::PDFDoc& document, JavascriptModifyItemInfo::JavascriptEventType event_type) {}
  virtual bool InitModifyItem(const pdf::PDFDoc &document, ModifyItemType item_type, int page_index, const WString &dict_name) {
      return true;
  }
  virtual void ResetModifyItem(const pdf::PDFDoc &document) { }
  virtual int GetVisiblePageCount(const pdf::PDFDoc& document) { return 0; }
  virtual int GetVisiblePage(const pdf::PDFDoc& document, int index) { return -1; }
  private:
    PDFDoc current_doc_;
};

void AddAnnotations(PDFPage &page)
{
  WString author = WString::FromLocal(g_author_str.c_str());

  if (ShouldAddType("line")) {
    // Add line annotation
    annots::Line line(page.AddAnnot(Annot::e_Line, GetRect(RectF(0,650,100,750))));
    line.SetFlags(g_flags);
    line.SetStartPoint(g_has_line_start ? g_line_start : PointF(20,650));
    line.SetEndPoint(g_has_line_end ? g_line_end : PointF(100,740));
    line.SetIntent(g_line_intent.c_str());
    line.SetContent(L"A line annotation");
    if (g_has_subject) line.SetSubject(WString::FromLocal(g_subject_str.c_str())); else line.SetSubject(L"Line");
    line.SetTitle(author);
    line.SetCreationDateTime(GetLocalDateTime());
    line.SetModifiedDateTime(GetLocalDateTime());
    line.SetUniqueID(WString::FromLocal(RandomUID()));
    if (g_has_content) line.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) line.SetBorderColor(g_color);
    if (g_has_border) line.SetBorderInfo(GetBorderInfo());
    line.ResetAppearanceStream();
    cout << "Add a line annotation." << endl;
  }

  if (ShouldAddType("circle")) {
    // Add circle annotation
    annots::Circle circle(page.AddAnnot(Annot::e_Circle, GetRect(RectF(100,650,200,750))));
    //This flag is used for printing annotations.
    circle.SetFlags(g_flags);
    circle.SetInnerRect(g_has_inner_rect ? g_inner_rect : RectF(120,660,160,740));
    if (g_has_fill_color) circle.SetFillColor(g_fill_color);
    if (g_has_subject) circle.SetSubject(WString::FromLocal(g_subject_str.c_str())); else circle.SetSubject(L"Circle");
    circle.SetTitle(author);
    circle.SetCreationDateTime(GetLocalDateTime());
    circle.SetModifiedDateTime(GetLocalDateTime());
    circle.SetUniqueID(WString::FromLocal(RandomUID()));
    if (g_has_content) circle.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) circle.SetBorderColor(g_color);
    if (g_has_border) circle.SetBorderInfo(GetBorderInfo());
    // Appearance should be reset.
    circle.ResetAppearanceStream();
    cout << "Add a circle annotation." << endl;
  }

  if (ShouldAddType("square")) {
    // Add square annotation
    annots::Square square(page.AddAnnot(Annot::e_Square, GetRect(RectF(200,650,300,750))));
    //This flag is used for printing annotations.
    square.SetFlags(g_flags);
    square.SetFillColor(g_has_fill_color ? g_fill_color : 0x00FF00);
    square.SetInnerRect(g_has_inner_rect ? g_inner_rect : RectF(220,660,260,740));
    if (g_has_subject) square.SetSubject(WString::FromLocal(g_subject_str.c_str())); else square.SetSubject(L"Square");
    square.SetTitle(author);
    square.SetCreationDateTime(GetLocalDateTime());
    square.SetModifiedDateTime(GetLocalDateTime());
    square.SetUniqueID(WString::FromLocal(RandomUID()));
    if (g_has_content) square.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) square.SetBorderColor(g_color);
    if (g_has_border) square.SetBorderInfo(GetBorderInfo());
    // Appearance should be reset.
    square.ResetAppearanceStream();
    cout << "Add a square annotation." << endl;
  }

  if (ShouldAddType("polygon")) {
    // Add polygon annotation
    annots::Polygon polygon(page.AddAnnot(Annot::e_Polygon, GetRect(RectF(300,650,500,750))));
    polygon.SetFlags(g_flags);
    if (g_has_border) {
      polygon.SetBorderInfo(GetBorderInfo());
    } else if (g_polygon_style == "dashed") {
      BorderInfo borderinfo;
      borderinfo.cloud_intensity = 2.0f;
      borderinfo.width = 2.0f;
      borderinfo.style = BorderInfo::e_Dashed;
      borderinfo.dash_phase = 3.f;
      borderinfo.dashes.SetSize(2);
      borderinfo.dashes.SetAt(0,2);
      borderinfo.dashes.SetAt(1,2);
      polygon.SetBorderInfo(borderinfo);
    } else {
      polygon.SetIntent("PolygonCloud");
    }
    polygon.SetFillColor(g_has_fill_color ? g_fill_color : 0x0000FF);
    if (g_has_vertexes) {
      polygon.SetVertexes(g_vertexes);
    } else {
      foxit::PointFArray vertexe_array;
      vertexe_array.Add(PointF(335,665));
      vertexe_array.Add(PointF(365,665));
      vertexe_array.Add(PointF(385,705));
      vertexe_array.Add(PointF(365,740));
      vertexe_array.Add(PointF(335,740));
      vertexe_array.Add(PointF(315,705));
      polygon.SetVertexes(vertexe_array);
    }
    if (g_has_subject) polygon.SetSubject(WString::FromLocal(g_subject_str.c_str())); else polygon.SetSubject(L"Polygon");
    polygon.SetTitle(author);
    polygon.SetCreationDateTime(GetLocalDateTime());
    polygon.SetModifiedDateTime(GetLocalDateTime());
    polygon.SetUniqueID(WString::FromLocal(RandomUID()));
    if (g_has_content) polygon.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) polygon.SetBorderColor(g_color);
    polygon.ResetAppearanceStream();
    cout << "Add a polygon annotation." << endl;
  }

  if (ShouldAddType("polyline")) {
    // Add polyline annotation
    annots::PolyLine polyline(page.AddAnnot(Annot::e_PolyLine, GetRect(RectF(500,650,600,700))));
    //This flag is used for printing annotations.
    polyline.SetFlags(g_flags);
    if (g_has_vertexes) {
      polyline.SetVertexes(g_vertexes);
    } else {
      foxit::PointFArray vertexe_array;
      vertexe_array.Add(PointF(515,705));
      vertexe_array.Add(PointF(535,740));
      vertexe_array.Add(PointF(565,740));
      vertexe_array.Add(PointF(585,705));
      vertexe_array.Add(PointF(565,665));
      vertexe_array.Add(PointF(535,665));
      polyline.SetVertexes(vertexe_array);
    }
    if (g_has_subject) polyline.SetSubject(WString::FromLocal(g_subject_str.c_str())); else polyline.SetSubject(L"PolyLine");
    polyline.SetTitle(author);
    polyline.SetCreationDateTime(GetLocalDateTime());
    polyline.SetModifiedDateTime(GetLocalDateTime());
    polyline.SetUniqueID(WString::FromLocal(RandomUID()));
    if (g_has_content) polyline.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) polyline.SetBorderColor(g_color);
    if (g_has_border) polyline.SetBorderInfo(GetBorderInfo());
    // Appearance should be reset.
    polyline.ResetAppearanceStream();
    cout << "Add a polyline annotation." << endl;
  }

  if (ShouldAddType("freetext")) {
    // Add freetext annotation. Type controlled by --freetext-intent, richtext by --richtext.
    annots::FreeText freetext(page.AddAnnot(Annot::e_FreeText, GetRect(RectF(10,550,200,600))));
    freetext.SetFlags(g_flags);
    foxit::pdf::DefaultAppearance default_ap;
    default_ap.flags = DefaultAppearance::e_FlagFont | DefaultAppearance::e_FlagFontSize | DefaultAppearance::e_FlagTextColor;
    default_ap.font = common::Font(common::Font::e_StdIDHelvetica);
    default_ap.text_size = g_has_da_font_size ? g_da_font_size : 12.0f;
    default_ap.text_color = g_has_da_text_color ? g_da_text_color : 0x000000;
    freetext.SetDefaultAppearance(default_ap);
    if (g_freetext_intent == "callout") {
      freetext.SetIntent("FreeTextCallout");
      PointFArray callout_points;
      if (g_has_callout_points) {
        callout_points = g_callout_points;
      } else {
        callout_points.Add(PointF(250,540));
        callout_points.Add(PointF(280,570));
        callout_points.Add(PointF(300,570));
      }
      freetext.SetCalloutLinePoints(callout_points);
      freetext.SetCalloutLineEndingStyle(annots::Markup::e_EndingStyleOpenArrow);
      freetext.SetContent(L"A callout annotation.");
      if (g_has_subject) freetext.SetSubject(WString::FromLocal(g_subject_str.c_str())); else freetext.SetSubject(L"FreeTextCallout");
    } else if (g_freetext_intent == "textbox") {
      freetext.SetContent(L"A text box annotation.");
      if (g_has_subject) freetext.SetSubject(WString::FromLocal(g_subject_str.c_str())); else freetext.SetSubject(L"Textbox");
    } else {
      freetext.SetIntent("FreeTextTypewriter");
      freetext.SetContent(L"A typewriter annotation");
      if (g_has_subject) freetext.SetSubject(WString::FromLocal(g_subject_str.c_str())); else freetext.SetSubject(L"FreeTextTypewriter");
    }
    freetext.SetTitle(author);
    freetext.SetCreationDateTime(GetLocalDateTime());
    freetext.SetModifiedDateTime(GetLocalDateTime());
    freetext.SetUniqueID(WString::FromLocal(RandomUID()));
    if (g_richtext) {
      RichTextStyle richtext_style;
      if (!g_rt_font.empty()) {
        richtext_style.font = Font(WString::FromLocal(g_rt_font.c_str()), 0, Font::e_CharsetANSI, 0);
      } else {
#if defined(__linux__)
        richtext_style.font = Font(L"FreeSerif", 0, Font::e_CharsetANSI, 0);
#else
        richtext_style.font = Font(L"Times New Roman", 0, Font::e_CharsetANSI, 0);
#endif
      }
      richtext_style.text_color = g_has_rt_text_color ? g_rt_text_color : 0xFF0000;
      richtext_style.text_size = g_has_rt_font_size ? g_rt_font_size : 10;
      freetext.AddRichText(L"Rich text content ", richtext_style);
      richtext_style.text_color = 0x00FF00;
      richtext_style.is_underline = true;
      freetext.AddRichText(L"underline ", richtext_style);
      if (!g_rt_font.empty()) {
        richtext_style.font = Font(WString::FromLocal(g_rt_font.c_str()), 0, Font::e_CharsetANSI, 0);
      } else {
#if defined(__linux__)
        richtext_style.font = Font(L"FreeSans", 0, Font::e_CharsetANSI, 0);
#else
        richtext_style.font = Font(L"Calibri", 0, Font::e_CharsetANSI, 0);
#endif
      }
      richtext_style.text_color = 0x0000FF;
      richtext_style.is_underline = false;
      richtext_style.is_strikethrough = true;
      int richtext_count = freetext.GetRichTextCount();
      freetext.InsertRichText(richtext_count - 1, L"strikethrough ", richtext_style);
    }
    if (g_has_content) freetext.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) freetext.SetBorderColor(g_color);
    if (g_has_border) freetext.SetBorderInfo(GetBorderInfo());
    freetext.ResetAppearanceStream();
    cout << "Add a freetext annotation." << endl;
  }

  if (ShouldAddType("highlight")) {
    // Add highlight annotation.
    annots::Highlight highlight(page.AddAnnot(Annot::e_Highlight, GetRect(RectF(10,450,100,550))));
    //This flag is used for printing annotations.
    highlight.SetFlags(g_flags);
    highlight.SetContent(L"Highlight");
    annots::QuadPoints quad_points;
    if (g_has_quad_points) {
      quad_points.first = g_quad_points_raw.GetAt(0);
      quad_points.second = g_quad_points_raw.GetAt(1);
      quad_points.third = g_quad_points_raw.GetAt(2);
      quad_points.fourth = g_quad_points_raw.GetAt(3);
    } else {
      quad_points.first = PointF(10, 500);
      quad_points.second = PointF(90, 500);
      quad_points.third = PointF(10, 480);
      quad_points.fourth = PointF(90, 480);
    }
    annots::QuadPointsArray quad_points_array;
    quad_points_array.Add(quad_points);
    highlight.SetQuadPoints(quad_points_array);
    if (g_has_subject) highlight.SetSubject(WString::FromLocal(g_subject_str.c_str())); else highlight.SetSubject(L"Highlight");
    highlight.SetTitle(author);
    highlight.SetCreationDateTime(GetLocalDateTime());
    highlight.SetModifiedDateTime(GetLocalDateTime());
    highlight.SetUniqueID(WString::FromLocal(RandomUID()));
    if (g_has_content) highlight.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) highlight.SetBorderColor(g_color);
    if (g_has_border) highlight.SetBorderInfo(GetBorderInfo());
    // Appearance should be reset.
    highlight.ResetAppearanceStream();
    cout << "Add a highlight annotation." << endl;
  }

  if (ShouldAddType("underline")) {
    // Add underline annotation.
    annots::Underline underline(page.AddAnnot(Annot::e_Underline, GetRect(RectF(100,450,200,550))));
    //This flag is used for printing annotations.
    underline.SetFlags(g_flags);
    annots::QuadPoints quad_points;
    if (g_has_quad_points) {
      quad_points.first = g_quad_points_raw.GetAt(0);
      quad_points.second = g_quad_points_raw.GetAt(1);
      quad_points.third = g_quad_points_raw.GetAt(2);
      quad_points.fourth = g_quad_points_raw.GetAt(3);
    } else {
      quad_points.first = PointF(110, 500);
      quad_points.second = PointF(190, 500);
      quad_points.third = PointF(110, 480);
      quad_points.fourth = PointF(190, 480);
    }
    annots::QuadPointsArray quad_points_array;
    quad_points_array.Add(quad_points);
    underline.SetQuadPoints(quad_points_array);
    if (g_has_subject) underline.SetSubject(WString::FromLocal(g_subject_str.c_str())); else underline.SetSubject(L"Underline");
    underline.SetTitle(author);
    underline.SetCreationDateTime(GetLocalDateTime());
    underline.SetModifiedDateTime(GetLocalDateTime());
    underline.SetUniqueID(WString::FromLocal(RandomUID()));
    if (g_has_content) underline.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) underline.SetBorderColor(g_color);
    if (g_has_border) underline.SetBorderInfo(GetBorderInfo());
    // Appearance should be reset.
    underline.ResetAppearanceStream();
    cout << "Add a underline annotation." << endl;
  }

  if (ShouldAddType("squiggly")) {
    // Add squiggly annotation.
    annots::Squiggly squiggly(page.AddAnnot(Annot::e_Squiggly, GetRect(RectF(200,450,300,550))));
    //This flag is used for printing annotations.
    squiggly.SetFlags(g_flags);
    squiggly.SetIntent("Squiggly");
    annots::QuadPoints quad_points;
    if (g_has_quad_points) {
      quad_points.first = g_quad_points_raw.GetAt(0);
      quad_points.second = g_quad_points_raw.GetAt(1);
      quad_points.third = g_quad_points_raw.GetAt(2);
      quad_points.fourth = g_quad_points_raw.GetAt(3);
    } else {
      quad_points.first = PointF(210, 500);
      quad_points.second = PointF(290, 500);
      quad_points.third = PointF(210, 480);
      quad_points.fourth = PointF(290, 480);
    }
    annots::QuadPointsArray quad_points_array;
    quad_points_array.Add(quad_points);
    squiggly.SetQuadPoints(quad_points_array);
    if (g_has_subject) squiggly.SetSubject(WString::FromLocal(g_subject_str.c_str())); else squiggly.SetSubject(L"Squiggly");
    squiggly.SetTitle(author);
    squiggly.SetCreationDateTime(GetLocalDateTime());
    squiggly.SetModifiedDateTime(GetLocalDateTime());
    squiggly.SetUniqueID(WString::FromLocal(RandomUID()));
    if (g_has_content) squiggly.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) squiggly.SetBorderColor(g_color);
    if (g_has_border) squiggly.SetBorderInfo(GetBorderInfo());
    // Appearance should be reset.
    squiggly.ResetAppearanceStream();
    cout << "Add a squiggly annotation." << endl;
  }

  if (ShouldAddType("strikeout")) {
    // Add strikeout annotation.
    annots::StrikeOut strikeout(page.AddAnnot(Annot::e_StrikeOut, GetRect(RectF(300,450,400,550))));
    //This flag is used for printing annotations.
    strikeout.SetFlags(g_flags);
    annots::QuadPoints quad_points;
    if (g_has_quad_points) {
      quad_points.first = g_quad_points_raw.GetAt(0);
      quad_points.second = g_quad_points_raw.GetAt(1);
      quad_points.third = g_quad_points_raw.GetAt(2);
      quad_points.fourth = g_quad_points_raw.GetAt(3);
    } else {
      quad_points.first = PointF(310, 500);
      quad_points.second = PointF(390, 500);
      quad_points.third = PointF(310, 480);
      quad_points.fourth = PointF(390, 480);
    }
    annots::QuadPointsArray quad_points_array;
    quad_points_array.Add(quad_points);
    strikeout.SetQuadPoints(quad_points_array);
    if (g_has_subject) strikeout.SetSubject(WString::FromLocal(g_subject_str.c_str())); else strikeout.SetSubject(L"StrikeOut");
    strikeout.SetTitle(author);
    strikeout.SetCreationDateTime(GetLocalDateTime());
    strikeout.SetModifiedDateTime(GetLocalDateTime());
    strikeout.SetUniqueID(WString::FromLocal(RandomUID()));
    if (g_has_content) strikeout.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) strikeout.SetBorderColor(g_color);
    if (g_has_border) strikeout.SetBorderInfo(GetBorderInfo());
    // Appearance should be reset.
    strikeout.ResetAppearanceStream();
    cout << "Add a strikeout annotation." << endl;
  }

  if (ShouldAddType("caret")) {
    // Add caret annotation.
    annots::Caret caret(page.AddAnnot(Annot::e_Caret, GetRect(RectF(400,450,420,470))));
    //This flag is used for printing annotations.
    caret.SetFlags(g_flags);
    caret.SetInnerRect(g_has_inner_rect ? g_inner_rect : RectF(410,450,430,470));
    caret.SetContent(L"Caret annotation");
    if (g_has_subject) caret.SetSubject(WString::FromLocal(g_subject_str.c_str())); else caret.SetSubject(L"Caret");
    caret.SetTitle(author);
    caret.SetCreationDateTime(GetLocalDateTime());
    caret.SetModifiedDateTime(GetLocalDateTime());
    caret.SetUniqueID(WString::FromLocal(RandomUID()));
    if (g_has_content) caret.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) caret.SetBorderColor(g_color);
    if (g_has_border) caret.SetBorderInfo(GetBorderInfo());
    // Appearance should be reset.
    caret.ResetAppearanceStream();
    cout << "Add a caret annotation." << endl;
  }

  if (ShouldAddType("note")) {
    // Add note annotation
    annots::Note note(page.AddAnnot(Annot::e_Note, GetRect(RectF(10,350,50,400))));
    //This flag is used for printing annotations.
    note.SetFlags(g_flags);
    note.SetIconName(g_icon_name.empty() ? "Comment" : g_icon_name.c_str());
    if (g_has_subject) note.SetSubject(WString::FromLocal(g_subject_str.c_str())); else note.SetSubject(L"Note");
    note.SetTitle(author);
    note.SetContent(L"Note annotation.");
    note.SetCreationDateTime(GetLocalDateTime());
    note.SetModifiedDateTime(GetLocalDateTime());
    note.SetUniqueID(WString::FromLocal(RandomUID()));
    // Add popup to note annotation
    if (g_popup) {
      RectF popup_rect = g_has_popup_rect ? g_popup_rect : RectF(300,450,500,550);
      Popup popup(page.AddAnnot(Annot::e_Popup, popup_rect));
      popup.SetBorderColor(g_has_popup_color ? g_popup_color : 0x00FF00);
      popup.SetOpenStatus(g_popup_open);
      popup.SetModifiedDateTime(GetLocalDateTime());
      note.SetPopup(popup);
    }
    // Add reply annotation to note annotation
    Note reply = note.AddReply();
    reply.SetContent(L"reply");
    reply.SetModifiedDateTime(GetLocalDateTime());
    reply.SetTitle(author);
    reply.SetUniqueID(WString::FromLocal(RandomUID()));
    // Add state annotation to note annotation
    Note state = note.AddStateAnnot(author, Markup::e_StateModelReview, Markup::e_StateAccepted);
    state.SetContent(L"Accepted set by Foxit SDK");
    state.SetUniqueID(WString::FromLocal(RandomUID()));
    if (g_has_content) note.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) note.SetBorderColor(g_color);
    // Appearance should be reset.
    note.ResetAppearanceStream();
    cout << "Add a note annotation." << endl;
  }

  if (ShouldAddType("ink")) {
    // Add ink annotation
    annots::Ink ink(page.AddAnnot(Annot::e_Ink, GetRect(RectF(100,350,200,450))));
    //This flag is used for printing annotations.
    ink.SetFlags(g_flags);
    foxit::common::Path inklist;
    if (g_has_ink_points) {
      inklist.MoveTo(g_ink_points.GetAt(0));
      for (int j = 1; j < g_ink_points.GetSize(); j++) {
        inklist.LineTo(g_ink_points.GetAt(j));
      }
    } else {
      float width = 100;
      float height = 100;
      float out_width = min(width, height) * 2 / 3.f;
      float inner_width = out_width * sin(18.f / 180.f * 3.14f) / sin(36.f / 180.f * 3.14f);
      PointF center(150, 400);
      float x = out_width;
      float y = 0;
      inklist.MoveTo(PointF(center.x + x, center.y + y));
      for (int i = 0; i < 5; i++) {
        x = out_width * cos(72.f * i / 180.f * 3.14f);
        y = out_width * sin(72.f * i / 180.f * 3.14f);
        inklist.LineTo(PointF(center.x + x, center.y + y));
        x = inner_width * cos((72.f * i + 36) / 180.f * 3.14f);
        y = inner_width * sin((72.f * i + 36) / 180.f * 3.14f);
        inklist.LineTo(PointF(center.x + x, center.y + y));
      }
      inklist.LineTo(PointF(center.x + out_width, center.y + 0));
      inklist.CloseFigure();
    }
    ink.SetInkList(inklist);
    if (g_has_subject) ink.SetSubject(WString::FromLocal(g_subject_str.c_str())); else ink.SetSubject(L"Ink");
    ink.SetTitle(author);
    ink.SetContent(L"Note annotation.");
    ink.SetCreationDateTime(GetLocalDateTime());
    ink.SetModifiedDateTime(GetLocalDateTime());
    ink.SetUniqueID(WString::FromLocal(RandomUID()));
    if (g_has_content) ink.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) ink.SetBorderColor(g_color);
    if (g_has_border) ink.SetBorderInfo(GetBorderInfo());
    // Appearance should be reset.
    ink.ResetAppearanceStream();
    cout << "Add an ink annotation." << endl;
  }

  if (ShouldAddType("fileattachment")) {
    // Add file attachment annotation
    WString pdf_file = g_attach_file.empty()
      ? (input_path + L"AboutFoxit.pdf")
      : WString::FromLocal(g_attach_file.c_str());
    annots::FileAttachment file_attachment(page.AddAnnot(Annot::e_FileAttachment, GetRect(RectF(280,350,300,380))));
    //This flag is used for printing annotations.
    file_attachment.SetFlags(g_flags);
    file_attachment.SetIconName("Graph");
    foxit::pdf::FileSpec file_spec = foxit::pdf::FileSpec(page.GetDocument());
    file_spec.SetFileName(WString::FromLocal(
      g_attach_name.empty() ? "attachment.pdf" : g_attach_name.c_str()));
    file_spec.SetCreationDateTime(GetLocalDateTime());
    file_spec.SetDescription(
      g_attach_desc.empty() ? "The original file" : g_attach_desc.c_str());
    file_spec.SetModifiedDateTime(GetLocalDateTime());
    file_spec.Embed(pdf_file);
    file_attachment.SetFileSpec(file_spec);
    if (g_has_subject) file_attachment.SetSubject(WString::FromLocal(g_subject_str.c_str())); else file_attachment.SetSubject(L"File Attachment");
    file_attachment.SetTitle(author);
    if (g_has_content) file_attachment.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) file_attachment.SetBorderColor(g_color);
    if (g_has_border) file_attachment.SetBorderInfo(GetBorderInfo());
    // Appearance should be reset.
    file_attachment.ResetAppearanceStream();
    cout << "Add an attachment annotation." << endl;
  }

  if (ShouldAddType("link")) {
    // Add link annotation
    annots::Link link(page.AddAnnot(Annot::e_Link, GetRect(RectF(350,350,380,400))));
    //This flag is used for printing annotations.
    link.SetFlags(g_flags);
    {
      Annot::HighlightingMode hm = Annot::e_HighlightingPush;
      if (g_link_highlight == "none") hm = Annot::e_HighlightingNone;
      else if (g_link_highlight == "invert") hm = Annot::e_HighlightingInvert;
      else if (g_link_highlight == "outline") hm = Annot::e_HighlightingOutline;
      else if (g_link_highlight == "toggle") hm = Annot::e_HighlightingToggle;
      link.SetHighlightingMode(hm);
    }
    // Add action for link annotation
    using foxit::pdf::actions::Action;
    using foxit::pdf::actions::URIAction;
    URIAction action = (URIAction)Action::Create(page.GetDocument(), Action::e_TypeURI);
    action.SetTrackPositionFlag(true);
    action.SetURI("www.foxitsoftware.com");
    link.SetAction(action);
    if (g_has_color) link.SetBorderColor(g_color);
    if (g_has_border) link.SetBorderInfo(GetBorderInfo());
    // Appearance should be reset.
    link.ResetAppearanceStream();
    cout << "Add a link annotation." << endl;
  }

  if (ShouldAddType("stamp")) {
    // Determine stamps directory: prefer --stamps-dir, then "Stamps/" next to the input file.
    foxit::String stamps_path;
    if (!g_stamps_dir.empty()) {
      stamps_path = foxit::String(g_stamps_dir.c_str());
    } else {
      stamps_path = foxit::String::FromUnicode(input_path + L"Stamps");
    }
    const char* icon = g_icon_name.empty() ? "Approved" : g_icon_name.c_str();
    // Check stamps directory is accessible by probing a known file.
    foxit::String probe = stamps_path;
    if (g_dynamic_stamp)
      probe = stamps_path + "/DynamicStamps/" + g_icon_name.c_str() + ".pdf";
    else
      probe = stamps_path + "/StaticStamps/" + g_icon_name.c_str() + ".pdf";

    FILE* test_dir = fopen((const char*)probe, "rb");
    if (!test_dir) {
      printf("[Warning] Stamps directory not accessible: %s. Skipping stamp annotations.\n", (const char*)stamps_path);
    } else {
      fclose(test_dir);
      IconProvider* icon_provider = IconProvider::Create(std::string(stamps_path, stamps_path.GetLength()));
      icon_provider->SetUseDynamicStamp(g_dynamic_stamp);
      Library::SetAnnotIconProviderCallback(icon_provider);
      if (g_dynamic_stamp) {
        Library::SetActionCallback(new ActionEvent(page.GetDocument()));
      } else {
        Library::SetActionCallback(NULL);
      }
      Stamp stamp(page.AddAnnot(Annot::e_Stamp, GetRect(RectF(10,150,100,250))));
      stamp.SetFlags(g_flags);
      stamp.SetIconName(icon);
      if (g_has_border) stamp.SetBorderInfo(GetBorderInfo());
      stamp.ResetAppearanceStream();
      cout << "Add a stamp annotation." << endl;
    }
  }

  if (ShouldAddType("screen")) {
    // Add screen annotation
    annots::Screen screen(page.AddAnnot(Annot::e_Screen, GetRect(RectF(300, 150, 400, 200))));
    screen.SetFlags(g_flags);
    screen.SetTitle(author);
    if (g_has_color) screen.SetBorderColor(g_color);
    if (g_has_border) screen.SetBorderInfo(GetBorderInfo());
    if (!g_screen_image.empty()) {
      Image image = Image(WString::FromLocal(g_screen_image.c_str()));
      screen.SetImage(image, 0, true);
    }
    if (!g_video_file.empty()) {
      WString video_file_path = WString::FromLocal(g_video_file.c_str());
      // Prepare rendition action
      using foxit::pdf::actions::Action;
      using foxit::pdf::actions::RenditionAction;
      RenditionAction rendition_action = RenditionAction(Action::Create(page.GetDocument(), Action::e_TypeRendition));
      rendition_action.SetOperationType(RenditionAction::e_OpTypeAssociate);
      rendition_action.SetScreenAnnot(screen);
      // Prepare rendition
      Rendition rendition(page.GetDocument());
      rendition.SetRenditionName(L"screen for rendition");
      WString video_name = WString::FromLocal(g_video_file.c_str());
      rendition.SetMediaClipName(video_name);
      rendition.SetPermission(Rendition::e_MediaPermTempAccess);
      FileSpec video_filespec(page.GetDocument());
      video_filespec.SetFileName(video_name);
      video_filespec.Embed(video_file_path);
      rendition.SetMediaClipFile(video_filespec);
      rendition.SetMediaClipContentType("video/mp4");
      rendition_action.InsertRendition(rendition);
      screen.SetAction(rendition_action);
    }
    screen.ResetAppearanceStream();
    cout << "Add a screen annotation." << endl;
  }
}

// Map type name string to Annot::Type enum for delete filtering.
static Annot::Type TypeNameToEnum(const std::string& name) {
  if (name == "line") return Annot::e_Line;
  if (name == "circle") return Annot::e_Circle;
  if (name == "square") return Annot::e_Square;
  if (name == "polygon") return Annot::e_Polygon;
  if (name == "polyline") return Annot::e_PolyLine;
  if (name == "freetext") return Annot::e_FreeText;
  if (name == "highlight") return Annot::e_Highlight;
  if (name == "underline") return Annot::e_Underline;
  if (name == "squiggly") return Annot::e_Squiggly;
  if (name == "strikeout") return Annot::e_StrikeOut;
  if (name == "caret") return Annot::e_Caret;
  if (name == "note") return Annot::e_Note;
  if (name == "ink") return Annot::e_Ink;
  if (name == "fileattachment") return Annot::e_FileAttachment;
  if (name == "link") return Annot::e_Link;
  if (name == "stamp") return Annot::e_Stamp;
  if (name == "screen") return Annot::e_Screen;
  return (Annot::Type)-1;
}

void DeleteAnnotations(PDFPage &page)
{
  // Build set of target annotation types.
  std::set<int> target_types;
  if (!g_types.empty() && g_types.count("all") == 0) {
    for (std::set<std::string>::iterator it = g_types.begin(); it != g_types.end(); ++it) {
      Annot::Type t = TypeNameToEnum(*it);
      if ((int)t != -1) target_types.insert((int)t);
    }
  }

  int count = page.GetAnnotCount();
  int removed = 0;
  // Iterate in reverse to safely remove by index.
  for (int i = count - 1; i >= 0; i--) {
    annots::Annot annot = page.GetAnnot(i);
    if (annot.IsEmpty()) continue;
    Annot::Type atype = annot.GetType();
    // If no type filter (all), or type matches filter, remove it.
    if (target_types.empty() || target_types.count((int)atype) > 0) {
      page.RemoveAnnot(annot);
      removed++;
    }
  }
  cout << "Removed " << removed << " annotation(s)." << endl;
}

void EditAnnotations(PDFPage &page)
{
  // Build set of target annotation types.
  std::set<int> target_types;
  if (!g_types.empty() && g_types.count("all") == 0) {
    for (std::set<std::string>::iterator it = g_types.begin(); it != g_types.end(); ++it) {
      Annot::Type t = TypeNameToEnum(*it);
      if ((int)t != -1) target_types.insert((int)t);
    }
  }

  int count = page.GetAnnotCount();
  int edited = 0;
  for (int i = 0; i < count; i++) {
    annots::Annot annot = page.GetAnnot(i);
    if (annot.IsEmpty()) continue;
    Annot::Type atype = annot.GetType();
    if (!target_types.empty() && target_types.count((int)atype) == 0) continue;

    // Apply common properties
    if (g_has_content) annot.SetContent(WString::FromLocal(g_content_str.c_str()));
    if (g_has_color) annot.SetBorderColor(g_color);
    if (g_has_border) annot.SetBorderInfo(GetBorderInfo());
    annot.SetFlags(g_flags);

    // Apply markup-specific properties
    if (annot.IsMarkup()) {
      annots::Markup markup(annot);
      if (g_has_subject) markup.SetSubject(WString::FromLocal(g_subject_str.c_str()));
      markup.SetTitle(WString::FromLocal(g_author_str.c_str()));
      markup.SetModifiedDateTime(GetLocalDateTime());
    }

    // Apply type-specific properties
    if (atype == Annot::e_Line) {
      annots::Line line(annot);
      if (g_has_line_start) line.SetStartPoint(g_line_start);
      if (g_has_line_end) line.SetEndPoint(g_line_end);
    } else if (atype == Annot::e_Circle) {
      annots::Circle circle(annot);
      if (g_has_inner_rect) circle.SetInnerRect(g_inner_rect);
      if (g_has_fill_color) circle.SetFillColor(g_fill_color);
    } else if (atype == Annot::e_Square) {
      annots::Square square(annot);
      if (g_has_inner_rect) square.SetInnerRect(g_inner_rect);
      if (g_has_fill_color) square.SetFillColor(g_fill_color);
    } else if (atype == Annot::e_Polygon) {
      annots::Polygon polygon(annot);
      if (g_has_vertexes) polygon.SetVertexes(g_vertexes);
      if (g_has_fill_color) polygon.SetFillColor(g_fill_color);
    } else if (atype == Annot::e_PolyLine) {
      annots::PolyLine polyline(annot);
      if (g_has_vertexes) polyline.SetVertexes(g_vertexes);
    } else if (atype == Annot::e_Caret) {
      annots::Caret caret(annot);
      if (g_has_inner_rect) caret.SetInnerRect(g_inner_rect);
    } else if (atype == Annot::e_Highlight) {
      if (g_has_quad_points) {
        annots::Highlight hl(annot);
        annots::QuadPoints qp;
        qp.first = g_quad_points_raw.GetAt(0);
        qp.second = g_quad_points_raw.GetAt(1);
        qp.third = g_quad_points_raw.GetAt(2);
        qp.fourth = g_quad_points_raw.GetAt(3);
        annots::QuadPointsArray qpa; qpa.Add(qp);
        hl.SetQuadPoints(qpa);
      }
    } else if (atype == Annot::e_Underline) {
      if (g_has_quad_points) {
        annots::Underline ul(annot);
        annots::QuadPoints qp;
        qp.first = g_quad_points_raw.GetAt(0);
        qp.second = g_quad_points_raw.GetAt(1);
        qp.third = g_quad_points_raw.GetAt(2);
        qp.fourth = g_quad_points_raw.GetAt(3);
        annots::QuadPointsArray qpa; qpa.Add(qp);
        ul.SetQuadPoints(qpa);
      }
    } else if (atype == Annot::e_Squiggly) {
      if (g_has_quad_points) {
        annots::Squiggly sq(annot);
        annots::QuadPoints qp;
        qp.first = g_quad_points_raw.GetAt(0);
        qp.second = g_quad_points_raw.GetAt(1);
        qp.third = g_quad_points_raw.GetAt(2);
        qp.fourth = g_quad_points_raw.GetAt(3);
        annots::QuadPointsArray qpa; qpa.Add(qp);
        sq.SetQuadPoints(qpa);
      }
    } else if (atype == Annot::e_StrikeOut) {
      if (g_has_quad_points) {
        annots::StrikeOut so(annot);
        annots::QuadPoints qp;
        qp.first = g_quad_points_raw.GetAt(0);
        qp.second = g_quad_points_raw.GetAt(1);
        qp.third = g_quad_points_raw.GetAt(2);
        qp.fourth = g_quad_points_raw.GetAt(3);
        annots::QuadPointsArray qpa; qpa.Add(qp);
        so.SetQuadPoints(qpa);
      }
    }

    if (g_has_rect) annot.Move(g_rect);
    annot.ResetAppearanceStream();
    edited++;
  }
  cout << "Edited " << edited << " annotation(s)." << endl;
}

static bool ParseArgs(int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    foxit::String arg(argv[i]);
    if (arg.Equal("--help")) {
      printf(
        "Usage: annotation_xxx [options]\n"
        "\n"
        "Required options: (none, all options have defaults and the tool runs without arguments)\n"
        "\n"
        "Optional options:\n"
        "  --op <operation>      Operation: add, delete, or edit             (default: add)\n"
        "  -i <file>             Input PDF file path            (default: annotation_input.pdf)\n"
        "  -o <file>             Output PDF file path           (default: annotation_output.pdf)\n"
        "  --password <pwd>      Password to open the PDF       (default: empty)\n"
        "  -p <index>            Page index to annotate, 0-based (default: 0)\n"
        "  --type <type>         Annotation type to add/delete; repeatable   (default: all)\n"
        "                        Values: all line circle square polygon polyline freetext highlight\n"
        "                                underline squiggly strikeout caret note ink link stamp screen\n"
        "  --flags <n>           Annotation flags integer        (default: 4 = Print)\n"
        "                        Common values: 1=Invisible 2=Hidden 4=Print 8=NoZoom\n"
        "                                       16=NoRotate 32=NoView 64=ReadOnly 128=Locked\n"
        "  --author <name>       Title/author for all annotations (default: Foxit SDK)\n"
        "  --content <text>      Content/comment text; overrides per-type default (default: per-type)\n"
        "  --subject <text>      Subject field; overrides per-type default   (default: per-type)\n"
        "  --rect <x1,y1,x2,y2> Bounding rect override for all annotations  (default: per-type)\n"
        "  --inner-rect <x1,y1,x2,y2> Inner rect for circle/square/caret    (default: per-type)\n"
        "  --vertexes <pts>      Vertexes for polygon/polyline: x1,y1;x2,y2;... (default: per-type)\n"
        "  --quad-points <pts>   QuadPoints for highlight/underline/squiggly/strikeout:\n"
        "                        x1,y1;x2,y2;x3,y3;x4,y4 (4 points required) (default: per-type)\n"
        "  --ink-points <pts>    Ink path points: x1,y1;x2,y2;... (at least 2) (default: star shape)\n"
        "  --color <RRGGBB>      Border/line color in hex, e.g. FF0000       (default: none)\n"
        "  --fill-color <RRGGBB> Fill color in hex (circle/square/polygon)   (default: none)\n"
        "  --border-width <n>    Border width for all annotations             (default: 1)\n"
        "  --border-style <v>    Border style: solid/dashed/underline/beveled/inset/cloudy (default: solid)\n"
        "  --border-intensity <n> Cloud intensity for cloudy border           (default: 0)\n"
        "  --line-start <x,y>    Line annotation start point                 (default: 20,650)\n"
        "  --line-end <x,y>      Line annotation end point                   (default: 100,740)\n"
        "  --line-intent <v>     LineArrow or Line                           (default: LineArrow)\n"
        "  --polygon-style <v>   cloud or dashed                              (default: cloud)\n"
        "  --freetext-intent <v> typewriter, callout, or textbox             (default: typewriter)\n"
        "  --da-font-size <n>    FreeText default appearance font size        (default: 12)\n"
        "  --da-text-color <HEX> FreeText default appearance text color       (default: 000000)\n"
        "  --callout-points <pts> Callout line points: x1,y1;x2,y2;x3,y3    (default: 250,540;280,570;300,570)\n"
        "  --richtext            Add richtext content to freetext annotation  (default: off)\n"
        "  --rt-font <name>      Richtext font name (default: Times New Roman / FreeSerif on Linux)\n"
        "  --rt-font-size <n>    Richtext font size                           (default: 10)\n"
        "  --rt-text-color <HEX> Richtext text color in RRGGBB               (default: FF0000)\n"
        "  --dynamic-stamp       Use dynamic stamp instead of static          (default: off)\n"
        "  --popup               Add popup to note annotation                  (default: on)\n"
        "  --no-popup            Do not add popup to note annotation\n"
        "  --popup-rect <x1,y1,x2,y2> Popup rect position                     (default: 300,450,500,550)\n"
        "  --popup-open          Set popup open status to true                 (default: closed)\n"
        "  --popup-color <HEX>   Popup border color in RRGGBB                  (default: 00FF00)\n"
        "  --icon <name>         Icon for note (default: Comment) or stamp   (default: Approved)\n"
        "  --stamps-dir <dir>    Stamps icon directory (default: Stamps/ next to input file)\n"
        "  --attach-file <path>  File to embed in file attachment annotation  (default: AboutFoxit.pdf next to input)\n"
        "  --attach-name <name>  Display filename for the attachment          (default: attachment.pdf)\n"
        "  --attach-desc <text>  Description of the attachment                (default: The original file)\n"
        "  --link-highlight <v>  Link highlighting mode: none/invert/outline/push/toggle (default: push)\n"
        "  --video-file <path>   Video file for screen annotation              (required for screen type)\n"
        "  --screen-image <path> Image for screen annotation appearance        (optional)\n"
        "  --help                Show this help and exit\n"
      );
      exit(0);
    }
#define NEED_VAL(opt) \
    do { if (i + 1 >= argc) { \
      printf("Missing value for '%s'.\nTry 'annotation_xxx --help' for more information.\n", (opt)); \
      return false; \
    } } while(0)

    if (arg.Equal("-i")) {
      NEED_VAL("-i"); ++i;
      g_input_path = argv[i];
    } else if (arg.Equal("--op")) {
      NEED_VAL("--op"); ++i;
      foxit::String v(argv[i]);
      if (!v.Equal("add") && !v.Equal("delete") && !v.Equal("edit")) {
        printf("Invalid --op '%s'. Expected: add, delete, or edit\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_operation = argv[i];
    } else if (arg.Equal("-o")) {
      NEED_VAL("-o"); ++i;
      g_output_path = argv[i];
    } else if (arg.Equal("-p")) {
      NEED_VAL("-p"); ++i;
      g_page_index = atoi(argv[i]);
    } else if (arg.Equal("--type")) {
      NEED_VAL("--type"); ++i;
      if (!IsValidType(argv[i])) {
        printf("Unknown type: '%s'.\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_types.insert(argv[i]);
    } else if (arg.Equal("--author")) {
      NEED_VAL("--author"); ++i;
      g_author_str = argv[i];
    } else if (arg.Equal("--flags")) {
      NEED_VAL("--flags"); ++i;
      g_flags = atoi(argv[i]);
    } else if (arg.Equal("--content")) {
      NEED_VAL("--content"); ++i;
      g_has_content = true; g_content_str = argv[i];
    } else if (arg.Equal("--subject")) {
      NEED_VAL("--subject"); ++i;
      g_has_subject = true; g_subject_str = argv[i];
    } else if (arg.Equal("--rect")) {
      NEED_VAL("--rect"); ++i;
      float x1, y1, x2, y2;
      if (!ParseXYZW(argv[i], &x1, &y1, &x2, &y2)) {
        printf("Invalid --rect '%s'. Expected: x1,y1,x2,y2\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_rect = true; g_rect = RectF(x1, y1, x2, y2);
    } else if (arg.Equal("--inner-rect")) {
      NEED_VAL("--inner-rect"); ++i;
      float x1, y1, x2, y2;
      if (!ParseXYZW(argv[i], &x1, &y1, &x2, &y2)) {
        printf("Invalid --inner-rect '%s'. Expected: x1,y1,x2,y2\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_inner_rect = true; g_inner_rect = RectF(x1, y1, x2, y2);
    } else if (arg.Equal("--vertexes")) {
      NEED_VAL("--vertexes"); ++i;
      if (!ParseVertexes(argv[i], g_vertexes)) {
        printf("Invalid --vertexes '%s'. Expected: x1,y1;x2,y2;x3,y3;... (at least 2 points)\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_vertexes = true;
    } else if (arg.Equal("--quad-points")) {
      NEED_VAL("--quad-points"); ++i;
      if (!ParseVertexes(argv[i], g_quad_points_raw) || g_quad_points_raw.GetSize() != 4) {
        printf("Invalid --quad-points '%s'. Expected: x1,y1;x2,y2;x3,y3;x4,y4 (exactly 4 points)\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_quad_points = true;
    } else if (arg.Equal("--ink-points")) {
      NEED_VAL("--ink-points"); ++i;
      if (!ParseVertexes(argv[i], g_ink_points)) {
        printf("Invalid --ink-points '%s'. Expected: x1,y1;x2,y2;... (at least 2 points)\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_ink_points = true;
    } else if (arg.Equal("--color")) {
      NEED_VAL("--color"); ++i;
      if (!ParseHexColor(argv[i], &g_color)) {
        printf("Invalid --color '%s'. Expected: RRGGBB hex (max 6 chars)\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_color = true;
    } else if (arg.Equal("--fill-color")) {
      NEED_VAL("--fill-color"); ++i;
      if (!ParseHexColor(argv[i], &g_fill_color)) {
        printf("Invalid --fill-color '%s'. Expected: RRGGBB hex (max 6 chars)\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_fill_color = true;
    } else if (arg.Equal("--border-width")) {
      NEED_VAL("--border-width"); ++i;
      g_border_width = (float)atof(argv[i]);
      g_has_border = true;
    } else if (arg.Equal("--border-style")) {
      NEED_VAL("--border-style"); ++i;
      foxit::String v(argv[i]);
      if (!v.Equal("solid") && !v.Equal("dashed") && !v.Equal("underline") && !v.Equal("beveled") && !v.Equal("inset") && !v.Equal("cloudy")) {
        printf("Invalid --border-style '%s'. Expected: solid, dashed, underline, beveled, inset, or cloudy\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_border_style = argv[i];
      g_has_border = true;
    } else if (arg.Equal("--border-intensity")) {
      NEED_VAL("--border-intensity"); ++i;
      g_border_intensity = (float)atof(argv[i]);
      g_has_border = true;
    } else if (arg.Equal("--line-start")) {
      NEED_VAL("--line-start"); ++i;
      float x, y;
      if (!ParseXY(argv[i], &x, &y)) {
        printf("Invalid --line-start '%s'. Expected: x,y\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_line_start = true; g_line_start = PointF(x, y);
    } else if (arg.Equal("--line-end")) {
      NEED_VAL("--line-end"); ++i;
      float x, y;
      if (!ParseXY(argv[i], &x, &y)) {
        printf("Invalid --line-end '%s'. Expected: x,y\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_line_end = true; g_line_end = PointF(x, y);
    } else if (arg.Equal("--line-intent")) {
      NEED_VAL("--line-intent"); ++i;
      foxit::String v(argv[i]);
      if (!v.Equal("LineArrow") && !v.Equal("Line")) {
        printf("Invalid --line-intent '%s'. Expected: LineArrow or Line\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_line_intent = argv[i];
    } else if (arg.Equal("--polygon-style")) {
      NEED_VAL("--polygon-style"); ++i;
      foxit::String v(argv[i]);
      if (!v.Equal("cloud") && !v.Equal("dashed")) {
        printf("Invalid --polygon-style '%s'. Expected: cloud or dashed\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_polygon_style = argv[i];
    } else if (arg.Equal("--freetext-intent")) {
      NEED_VAL("--freetext-intent"); ++i;
      foxit::String v(argv[i]);
      if (!v.Equal("typewriter") && !v.Equal("callout") && !v.Equal("textbox")) {
        printf("Invalid --freetext-intent '%s'. Expected: typewriter, callout, or textbox\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_freetext_intent = true;
      g_freetext_intent = argv[i];
    } else if (arg.Equal("--da-font-size")) {
      NEED_VAL("--da-font-size"); ++i;
      g_da_font_size = (float)atof(argv[i]);
      g_has_da_font_size = true;
    } else if (arg.Equal("--da-text-color")) {
      NEED_VAL("--da-text-color"); ++i;
      if (!ParseHexColor(argv[i], &g_da_text_color)) {
        printf("Invalid --da-text-color '%s'. Expected: RRGGBB hex\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_da_text_color = true;
    } else if (arg.Equal("--callout-points")) {
      NEED_VAL("--callout-points"); ++i;
      if (!ParseVertexes(argv[i], g_callout_points) || g_callout_points.GetSize() < 2) {
        printf("Invalid --callout-points '%s'. Expected: x1,y1;x2,y2[;x3,y3] (2-3 points)\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_callout_points = true;
    } else if (arg.Equal("--richtext")) {
      g_richtext = true;
    } else if (arg.Equal("--rt-font")) {
      NEED_VAL("--rt-font"); ++i;
      g_rt_font = argv[i];
    } else if (arg.Equal("--rt-font-size")) {
      NEED_VAL("--rt-font-size"); ++i;
      g_rt_font_size = (float)atof(argv[i]);
      g_has_rt_font_size = true;
    } else if (arg.Equal("--rt-text-color")) {
      NEED_VAL("--rt-text-color"); ++i;
      if (!ParseHexColor(argv[i], &g_rt_text_color)) {
        printf("Invalid --rt-text-color '%s'. Expected: RRGGBB hex\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_rt_text_color = true;
    } else if (arg.Equal("--dynamic-stamp")) {
      g_dynamic_stamp = true;
    } else if (arg.Equal("--popup")) {
      g_popup = true;
    } else if (arg.Equal("--no-popup")) {
      g_popup = false;
    } else if (arg.Equal("--popup-rect")) {
      NEED_VAL("--popup-rect"); ++i;
      float x1, y1, x2, y2;
      if (!ParseXYZW(argv[i], &x1, &y1, &x2, &y2)) {
        printf("Invalid --popup-rect '%s'. Expected: x1,y1,x2,y2\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_popup_rect = true; g_popup_rect = RectF(x1, y1, x2, y2);
    } else if (arg.Equal("--popup-open")) {
      g_popup_open = true;
    } else if (arg.Equal("--popup-color")) {
      NEED_VAL("--popup-color"); ++i;
      if (!ParseHexColor(argv[i], &g_popup_color)) {
        printf("Invalid --popup-color '%s'. Expected: RRGGBB hex\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_has_popup_color = true;
    } else if (arg.Equal("--icon")) {
      NEED_VAL("--icon"); ++i;
      g_icon_name = argv[i];
    } else if (arg.Equal("--stamps-dir")) {
      NEED_VAL("--stamps-dir"); ++i;
      g_stamps_dir = argv[i];
    } else if (arg.Equal("--attach-file")) {
      NEED_VAL("--attach-file"); ++i;
      g_attach_file = argv[i];
    } else if (arg.Equal("--attach-name")) {
      NEED_VAL("--attach-name"); ++i;
      g_attach_name = argv[i];
    } else if (arg.Equal("--attach-desc")) {
      NEED_VAL("--attach-desc"); ++i;
      g_attach_desc = argv[i];
    } else if (arg.Equal("--link-highlight")) {
      NEED_VAL("--link-highlight"); ++i;
      foxit::String v(argv[i]);
      if (!v.Equal("none") && !v.Equal("invert") && !v.Equal("outline") && !v.Equal("push") && !v.Equal("toggle")) {
        printf("Invalid --link-highlight '%s'. Expected: none, invert, outline, push, or toggle\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
        return false;
      }
      g_link_highlight = argv[i];
    } else if (arg.Equal("--video-file")) {
      NEED_VAL("--video-file"); ++i;
      g_video_file = argv[i];
    } else if (arg.Equal("--screen-image")) {
      NEED_VAL("--screen-image"); ++i;
      g_screen_image = argv[i];
    } else if (arg.Equal("--password")) {
      NEED_VAL("--password"); ++i;
      g_password = argv[i];
    } else {
      printf("Unknown argument: '%s'.\nTry 'annotation_xxx --help' for more information.\n", argv[i]);
      return false;
    }
#undef NEED_VAL
  }
  return true;
}

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
    WString input_file = WString::FromLocal(g_input_path.c_str());
    // Derive input directory for auxiliary files (AboutFoxit.pdf, FoxitLogo.jpg, etc.)
    {
      std::size_t sep = g_input_path.find_last_of("/\\");
      std::string dir = (sep != std::string::npos) ? g_input_path.substr(0, sep + 1) : std::string("./");
      input_path = WString::FromLocal(dir.c_str());
    }
    WString output_file = WString::FromLocal(g_output_path.c_str());
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
    // Load a document
    PDFDoc doc = PDFDoc(input_file);
    doc.Load(foxit::String(g_password.c_str()));
    PDFPage page = doc.GetPage(g_page_index);
    if (g_operation == "delete") {
      DeleteAnnotations(page);
    } else if (g_operation == "edit") {
      EditAnnotations(page);
    } else {
      AddAnnotations(page);
    }
    // Save PDF file
    doc.SaveAs(output_file, PDFDoc::e_SaveFlagNoOriginal);
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
