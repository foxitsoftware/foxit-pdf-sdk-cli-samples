// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to add, sign, verify and get PAdES level of
// a PAdES signature in PDF document.

// Include header files.
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <time.h>
#if defined(_WIN32) || defined(_WIN64)
#include<direct.h>
#else
#include <sys/stat.h>
#endif

// Include Foxit SDK header files.
#include "../../../include/pdf/annots/fs_annot.h"
#include "../../../include/common/fs_image.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/fs_signature.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace objects;
using namespace file;

enum PadesMode {
  e_ModeSign = 0,
  e_ModeVerify,
  e_ModeSignVerify
};

struct PadesCommand {
  WString input_file;
  WString output_file;
  Signature::PAdESLevel level;
  PadesMode mode;
  WString cert_path;
  WString cert_password;
  Signature::DigestAlgorithm digest_algorithm;
  WString tsa_name;
  WString tsa_url;
  int sig_index;
  WString signer;
  WString contact_info;
  WString dn;
  WString location;
  WString reason;
  bool has_sig_index;
  bool has_input;
  bool has_output;

  PadesCommand()
      : level(Signature::e_PAdESLevelBB),
        mode(e_ModeSignVerify),
        digest_algorithm(Signature::e_DigestSHA256),
        sig_index(-1),
        has_sig_index(false),
        has_input(false),
        has_output(false) {}
};

void PrintUsage() {
  cout << "Usage:" << endl
      << "pades --input <input.pdf> --output <output.pdf> --cert <cert.pfx> --cert-password <pwd> [options]" << endl
      << endl
      << "Required:" << endl
      << "--input <path>                 Input PDF path." << endl
      << "--output <path>                Output signed PDF path." << endl
      << "--cert <path>                  PFX certificate path for signing." << endl
      << "--cert-password <text>         Certificate password." << endl
      << endl
      << "Optional:" << endl
      << "--mode <sign|verify|sign-verify>" << endl
      << "--level <B|BT|BLT|BLTA>        Expected/signing PAdES level." << endl
      << "--digest <sha1|sha256|sha384|sha512>  Digest algorithm (default: sha256)." << endl
      << "--tsa-name <text>              TSA server display name." << endl
      << "--tsa-url <url>                TSA server URL." << endl
      << "--sig-index <int>              Verify one signature index, default all." << endl
      << "--signer <name>                Signer name (default: \"Foxit PDF SDK\")." << endl
      << "--contact <info>               Contact info (default: \"support@foxitsoftware.com\")." << endl
      << "--dn <string>                  DN string (default: \"CN=CN,MAIL=MAIL@MAIL.COM\")." << endl
      << "--location <string>            Location (default: \"Fuzhou, China\")." << endl
      << "--reason <string>              Reason for signing (default: auto-generated)." << endl;
}

bool ParseIntValue(const String& value, int& out_value) {
  char* end_ptr = NULL;
  out_value = static_cast<int>(strtol((const char*)value, &end_ptr, 10));
  return end_ptr != NULL && *end_ptr == '\0';
}

bool ParseLevel(const String& value, Signature::PAdESLevel& level) {
  if (value.Equal("B")) {
    level = Signature::e_PAdESLevelBB;
    return true;
  }
  if (value.Equal("BT")) {
    level = Signature::e_PAdESLevelBT;
    return true;
  }
  if (value.Equal("BLT")) {
    level = Signature::e_PAdESLevelBLT;
    return true;
  }
  if (value.Equal("BLTA")) {
    level = Signature::e_PAdESLevelBLTA;
    return true;
  }
  return false;
}

bool ParseDigestAlgorithm(const String& digest, Signature::DigestAlgorithm& out) {
  if (digest.Equal("sha1")) { out = Signature::e_DigestSHA1; return true; }
  if (digest.Equal("sha256")) { out = Signature::e_DigestSHA256; return true; }
  if (digest.Equal("sha384")) { out = Signature::e_DigestSHA384; return true; }
  if (digest.Equal("sha512")) { out = Signature::e_DigestSHA512; return true; }
  return false;
}

bool ParseMode(const String& value, PadesMode& mode) {
  if (value.Equal("sign")) {
    mode = e_ModeSign;
    return true;
  }
  if (value.Equal("verify")) {
    mode = e_ModeVerify;
    return true;
  }
  if (value.Equal("sign-verify")) {
    mode = e_ModeSignVerify;
    return true;
  }
  return false;
}

bool FileExists(const WString& path) {
  FILE* file = NULL;
#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&file, String::FromUnicode(path), "rb");
#else
  file = fopen(String::FromUnicode(path), "rb");
#endif
  if (file == NULL) {
    return false;
  }
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
  if (directory.IsEmpty()) {
    return;
  }
#if defined(_WIN32) || defined(_WIN64)
  _mkdir(String::FromUnicode(directory));
#else
  mkdir(String::FromUnicode(directory), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

bool AnalysisParameter(int argc, char* argv[], PadesCommand& command) {
  if (argc < 5 || ((argc - 1) % 2 != 0)) {
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
    } else if (key.Equal("--level")) {
      if (!ParseLevel(value, command.level)) {
        return false;
      }
    } else if (key.Equal("--mode")) {
      if (!ParseMode(value, command.mode)) {
        return false;
      }
    } else if (key.Equal("--cert")) {
      command.cert_path = WString::FromUTF8(value);
    } else if (key.Equal("--cert-password")) {
      command.cert_password = WString::FromUTF8(value);
    } else if (key.Equal("--digest")) {
      if (!ParseDigestAlgorithm(value, command.digest_algorithm)) {
        return false;
      }
    } else if (key.Equal("--signer")) {
      command.signer = WString::FromUTF8(value);
    } else if (key.Equal("--contact")) {
      command.contact_info = WString::FromUTF8(value);
    } else if (key.Equal("--dn")) {
      command.dn = WString::FromUTF8(value);
    } else if (key.Equal("--location")) {
      command.location = WString::FromUTF8(value);
    } else if (key.Equal("--reason")) {
      command.reason = WString::FromUTF8(value);
    } else if (key.Equal("--tsa-name")) {
      command.tsa_name = WString::FromUTF8(value);
    } else if (key.Equal("--tsa-url")) {
      command.tsa_url = WString::FromUTF8(value);
    } else if (key.Equal("--sig-index")) {
      if (!ParseIntValue(value, command.sig_index) || command.sig_index < 0) {
        return false;
      }
      command.has_sig_index = true;
    } else {
      return false;
    }
  }

  return command.has_input && command.has_output;
}

// sn and key information from Foxit PDF SDK's key files are used to initialize Foxit PDF SDK library. 
static const char* sn = "VQv/htdRKu1rjhk90MTKF+/aCs9DfQRtBF4SNc0GwPlOX8ualGeTJg==";
static const char* key = "ezJvj18mtBp399sXJVWofHfLliq8pf9v7BMskv+zLerFJVWviGEfZ+kCs+nfwjfVTthVUzkIj1qr9k37wnS1f2YSW3jOA/0EfK806dB9bigN097uSthk6eeafc30XHVMSGerLpzYziDW9zL/1oMTZ80YX6Trs0BWaif611VZm/SJ4PkaRzN0sCCOUUKXl8+dUun4a2hGn1fxneYEh8ROSeTXRbNA0ltpL6d4xs+X4o5p+sJBSDjgoProtzbAeTELgzieFZtk/2qbfUzT0Zi3hIIFde/UHOEemCp88Une4k87YS8hK3n5H0s1EJLW8P4QUKdYIFU+r6iLZ3SVrT61l1IBvE0DoO+cUd3GpcA3XnAUamIDwtLzg6c7atTPKDnb3lM25K+LB7/1HIUTMZHmvE2JCuQM7J/c7hkYGoD4KvKl3UIgbGYCyEjQabHNVE0FiSWX7ZlE2RAA6DlzIxWhwhAbz4pkVggWTesZP+vVclWC2Vypebbt2zx+qTVWMNQTsXD9MKUMdOqRAvuytH/Hqw9LE3AoTR6oJE9FWw6NzkXAS2k0ylHHE7kdJh3z3MrCHblR3z1T28O1Me8WbGl1+1WaVEod5gowIsn/ut1/2MLYzyo3DCgDkNr+2nzeZSKIx9J92z6mEwexxlTfOea8cabcYO4QLbyCb+CNq5EhqsVv6UVMDkex6s9YA26WEjj4/Vp8hCKUBavqqqFOJtg8m7EDQQ9G0ZtfX4gLuQulsbluuxynUHrIF0BLdKfKLdAZav3+9lFiA2ORRwLWysYqASdCv4jE4oj2zpTIq4t4iFeOx2/PwJ8fuTMHZkihAbeKolrFq9MZCh6kUg0o1M6kowt8AHPhSG0I1Ng/OCz1lA6m2eBWu2dXtL4P7OJCs447MYgHPhc9k/K+LJvfzOBovIfogZKdwEWbN86ItHVQTDLs3Qlx0qn4tpTJfBcZJUpi03satDiaipWhVNznvr9kDKlGx1VBu+ciqB8MDaC/MGAVeRl+ByqT5t2L6I8koG58o/sNRjMZ1Y5CfFIl5J7j3J2+acYW9vPNrkLgbD5DPAQGhJUlo2K59v62/HrEJDXzz2ue6FEbIASS37X9JdBWqoMav5Ti1dlNpDONbprffNqVyiGromUvTGT4Y9KJWqBCou/p1KzW9s4rCuwzONuKNF7nOqx0lGsCQrL8pkvDwDKJ94lx5oyHZdHNS2xMh98qPo1V7nc=";

#if defined(_WIN32) || defined(_WIN64)
static WString input_path = WString::FromLocal("../input_files/");
#else
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

#define TIMESTAMP_SIGNATURE_FILTER "Adobe.PPKLite"
#define TIMESTAMP_SIGNATURE_SUBFILTER "ETSI.RFC3161"
#define FREE_ETSIRFC316TSA_SERVER_NAME L"FreeTSAServer"
#define FREE_ETSIRFC316TSA_SERVER_URL L"http://ca.signfiles.com/TSAServer.aspx"

#define ETSICADES_SIGNATURE_FILTER "Adobe.PPKLite"
#define ETSICADES_SIGNATURE_SUBFILTER "ETSI.CAdES.detached"


#if !defined(WIN32) && !defined(_WIN64)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#endif

string TransformSignatureStateToString(uint32 sig_state) {
  string state_str;
  if (sig_state & Signature::e_TypeUnknown)
    state_str += "Unknown";
  if (sig_state & Signature::e_StateNoSignData) {
    if (state_str.length()>0) state_str += "|";
    state_str += "NoSignData";
  }
  if (sig_state & Signature::e_StateUnsigned) {
    if (state_str.length()>0) state_str += "|";
    state_str += "Unsigned";
  }
  if (sig_state & Signature::e_StateSigned) {
    if (state_str.length()>0) state_str += "|";
    state_str += "Signed";
  }
  if (sig_state & Signature::e_StateVerifyValid) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerfiyValid";
  }
  if (sig_state & Signature::e_StateVerifyInvalid) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyInvalid";
  }
  if (sig_state & Signature::e_StateVerifyErrorData) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyErrorData";
  }
  if (sig_state & Signature::e_StateVerifyNoSupportWay) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyNoSupportWay";
  }
  if (sig_state & Signature::e_StateVerifyErrorByteRange) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyErrorByteRange";
  }
  if (sig_state & Signature::e_StateVerifyChange) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyChange";
  }
  if (sig_state & Signature::e_StateVerifyIncredible) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyIncredible";
  }
  if (sig_state & Signature::e_StateVerifyNoChange) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyNoChange";
  }
  if (sig_state & Signature::e_StateVerifyIssueValid) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyIssueValid";
  }
  if (sig_state & Signature::e_StateVerifyIssueUnknown) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyIssueUnknown";
  }
  if (sig_state & Signature::e_StateVerifyIssueRevoke) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyIssueRevoke";
  }
  if (sig_state & Signature::e_StateVerifyIssueExpire) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyIssueExpire";
  }
  if (sig_state & Signature::e_StateVerifyIssueUncheck) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyIssueUncheck";
  }
  if (sig_state & Signature::e_StateVerifyIssueCurrent) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyIssueCurrent";
  }
  if (sig_state & Signature::e_StateVerifyTimestampNone) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyTimestampNone";
  }
  if (sig_state & Signature::e_StateVerifyTimestampDoc) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyTimestampDoc";
  }
  if (sig_state & Signature::e_StateVerifyTimestampValid) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyTimestampValid";
  }
  if (sig_state & Signature::e_StateVerifyTimestampInvalid) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyTimestampInvalid";
  }
  if (sig_state & Signature::e_StateVerifyTimestampExpire) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyTimestampExpire";
  }
  if (sig_state & Signature::e_StateVerifyTimestampIssueUnknown) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyTimestampIssueUnknown";
  }
  if (sig_state & Signature::e_StateVerifyTimestampIssueValid) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyTimestampIssueValid";
  }
  if (sig_state & Signature::e_StateVerifyTimestampTimeBefore) {
    if (state_str.length()>0) state_str += "|";
    state_str += "VerifyTimestampTimeBefore";
  }
  if (sig_state & Signature::e_StateCertCannotGetVRI) {
    if (state_str.length()>0) state_str += "|";
    state_str += "CertCannotGetVRI";
  }
  return state_str;
}

WString GenerateNewPDFFileName(const WString& org_file_name, const wchar_t* suffix_name) {
  FX_STRSIZE org_length = org_file_name.GetLength();
  WString first_part = org_file_name.Left(org_length - 4);
  WString new_file_path = first_part + suffix_name + L".pdf";
  return new_file_path;
}

DateTime GetLocalDateTime() {
  time_t t = time(NULL);
#if (WINAPI_PARTITION_APP || WINAPI_PARTITION_PC_APP) || \
  (defined(_WIN32) || defined(_WIN64)) && _MSC_VER != 1200
  struct tm _Tm;
  localtime_s(&_Tm, &t);
  struct tm* system_time = &_Tm;
  _tzset();
  long time_zone = NULL;
  _get_timezone(&time_zone);
  int timezone_hour = time_zone / 3600 * -1;
  int timezone_minute = (abs(time_zone) % 3600) / 60;
#elif defined(__linux__)
  struct tm* system_time = localtime(&t);
  tzset();
  int timezone_hour = __timezone / 3600 * -1;
  int timezone_minute = ((int)abs(__timezone) % 3600) / 60;
#elif defined(__APPLE__)
  struct tm* system_time = localtime(&t);
  tzset();
  int timezone_hour = timezone / 3600 * -1;
  int timezone_minute = ((int)abs(timezone) % 3600) / 60;
#endif
  DateTime datetime;
  datetime.year = static_cast<uint16>(system_time->tm_year + 1900);
  datetime.month = static_cast<uint16>(system_time->tm_mon + 1);
  datetime.day = static_cast<uint16>(system_time->tm_mday);
  datetime.hour = static_cast<uint16>(system_time->tm_hour);
  datetime.minute = static_cast<uint16>(system_time->tm_min);
  datetime.second = static_cast<uint16>(system_time->tm_sec);
  datetime.utc_hour_offset = timezone_hour;
  datetime.utc_minute_offset = timezone_minute;

  return datetime;
}

const wchar_t* TransformLevel2WideString(Signature::PAdESLevel level) {
  switch (level) {
    case Signature::e_PAdESLevelNotPAdES:
      return L"NotPades";
    case Signature::e_PAdESLevelNone:
      return L"NoneLevel";
    case Signature::e_PAdESLevelBB:
      return L"LevelB";
    case Signature::e_PAdESLevelBT:
      return L"LevelT";
    case Signature::e_PAdESLevelBLT:
      return L"LevelLT";
    case Signature::e_PAdESLevelBLTA:
      return L"LevelLTA";
    default:
      return L"Unknown level value";
  }
}

const char* TransformLevel2String(Signature::PAdESLevel level) {
  switch (level) {
    case Signature::e_PAdESLevelNotPAdES:
      return "NotPades";
    case Signature::e_PAdESLevelNone:
      return "NoneLeve";
    case Signature::e_PAdESLevelBB:
      return "LevelB";
    case Signature::e_PAdESLevelBT:
      return "LevelT";
    case Signature::e_PAdESLevelBLT:
      return "LevelLT";
    case Signature::e_PAdESLevelBLTA:
      return "LevelLTA";
    default:
      return "Unknown level value";
  }
}

void PAdESSign(const WString& input_pdf_path, const WString& signed_pdf_path, const PadesCommand& command) {
  printf("To add a PAdES signature in %s\r\n", TransformLevel2String(command.level));
  WString cached_signed_pdf_path = signed_pdf_path;
  WString real_signed_pdf_path = signed_pdf_path;
  if (command.level > Signature::e_PAdESLevelBT) {
    cached_signed_pdf_path = signed_pdf_path.Left(signed_pdf_path.GetLength() - 4) + L"_cache.pdf";
  }

  PDFDoc pdf_doc(input_pdf_path);
  pdf_doc.StartLoad();
  PDFPage pdf_page = pdf_doc.GetPage(0);
  float page_width = pdf_page.GetWidth();
  float page_height = pdf_page.GetHeight();
  RectF new_sig_rect((page_width / 2 - 50.0f), (page_height / 2 - 50.0f), (page_width / 2 + 50.0f), (page_height / 2 + 50.0f));
  Signature new_signature = pdf_page.AddSignature(new_sig_rect);
  new_signature.SetFilter(ETSICADES_SIGNATURE_FILTER);
  new_signature.SetSubFilter(ETSICADES_SIGNATURE_SUBFILTER);

  WString signer = command.signer.IsEmpty() ? L"Foxit PDF SDK" : command.signer;
  WString contact_info = command.contact_info.IsEmpty() ? L"support@foxitsoftware.com" : command.contact_info;
  WString dn = command.dn.IsEmpty() ? L"CN=CN,MAIL=MAIL@MAIL.COM" : command.dn;
  WString location = command.location.IsEmpty() ? L"Fuzhou, China" : command.location;
  new_signature.SetKeyValue(Signature::e_KeyNameSigner, signer);
  new_signature.SetKeyValue(Signature::e_KeyNameContactInfo, contact_info);
  new_signature.SetKeyValue(Signature::e_KeyNameDN, dn);
  new_signature.SetKeyValue(Signature::e_KeyNameLocation, location);
  String new_value;
  if (command.reason.IsEmpty()) {
    new_value.Format("As a sample for subfilter \"%s\", in %s ", ETSICADES_SIGNATURE_SUBFILTER, TransformLevel2String(command.level));
  } else {
    new_value = String((const char*)String::FromUnicode(command.reason));
  }
  new_signature.SetKeyValue(Signature::e_KeyNameReason, (const wchar_t*)WString::FromLocal(new_value));
  new_signature.SetKeyValue(Signature::e_KeyNameText, (const wchar_t*)WString::FromLocal(new_value));
  DateTime sign_time = GetLocalDateTime();
  new_signature.SetSignTime(sign_time);
  // Set appearance flags to decide which content would be used in appearance.
  uint32 ap_flags = Signature::e_APFlagLabel | Signature::e_APFlagSigner |
    Signature::e_APFlagReason | Signature::e_APFlagDN |
    Signature::e_APFlagLocation | Signature::e_APFlagText |
    Signature::e_APFlagSigningTime ;
  new_signature.SetAppearanceFlags(ap_flags);

  Progressive sign_progressive = new_signature.StartSign(command.cert_path, command.cert_password, command.digest_algorithm, cached_signed_pdf_path);
  if (sign_progressive.GetRateOfProgress() != 100) {
    if (Progressive::e_Finished != sign_progressive.Continue()) {
      printf("[Failed] Fail to sign the new CAdES signature.\r\n");
      return;
    }
  }

  if (command.level > Signature::e_PAdESLevelBT) {
    PDFDoc cache_pdf_doc(cached_signed_pdf_path);
    cache_pdf_doc.StartLoad();
    // Here, we only simply create an empty DSS object in PDF document, just as a simple exmaple.
    // In fact, user should use LTVVerifier to add DSS.
    cache_pdf_doc.CreateDSS();

    if (command.level > Signature::e_PAdESLevelBLT) {
      PDFPage cache_pdf_page = cache_pdf_doc.GetPage(0);
      Signature time_stamp_signature = cache_pdf_page.AddSignature(RectF(), L"", Signature::e_SignatureTypeTimeStamp);
      Progressive sign_ts_progressive = time_stamp_signature.StartSign(L"", L"", Signature::e_DigestSHA256, real_signed_pdf_path);
      if (sign_ts_progressive.GetRateOfProgress() != 100) {
        if (Progressive::e_Finished != sign_ts_progressive.Continue()) {
          printf("[Failed] Fail to sign the new time stamp signature.\r\n");
          return;
        }
      }
    } else {
      cache_pdf_doc.SaveAs(real_signed_pdf_path, PDFDoc::e_SaveFlagIncremental);
    }
  }
}

void PAdESVerify(const WString& check_pdf_path, Signature::PAdESLevel expect_pades_level, int sig_index) {
  PDFDoc check_pdf_doc(check_pdf_path);
  check_pdf_doc.StartLoad();
  printf("To verify level of PAdES signature in file %s\r\n", (const char*)String::FromUnicode(check_pdf_path));

  int sig_count = check_pdf_doc.GetSignatureCount();
  if (0 == sig_count)
    printf("No signature in current PDF file.\r\n");
  if (sig_index >= sig_count) {
    printf("Signature index %d is out of range.\r\n", sig_index);
    return;
  }
  bool has_cades_signature = false;
  int begin = (sig_index >= 0) ? sig_index : 0;
  int end = (sig_index >= 0) ? sig_index + 1 : check_pdf_doc.GetSignatureCount();
  for (int i = begin; i < end; i++) {
    Signature temp_sig = check_pdf_doc.GetSignature(i);
    if (temp_sig.IsEmpty()) continue;
    uint32 sig_org_state = temp_sig.GetState();
    bool is_true = sig_org_state & Signature::e_StateSigned;
    if (!is_true) continue;
    if (temp_sig.GetSubFilter() == ETSICADES_SIGNATURE_SUBFILTER) {
      has_cades_signature = true;
      // Verify PAdES signature.
      Progressive verify_progressive = temp_sig.StartVerify();
      if (100 != verify_progressive.GetRateOfProgress()) {
        if (Progressive::e_Finished != verify_progressive.Continue()) {
          printf("[Failed] Fail to verify a PAdES signature. Signature index:%d\r\n", i);
          continue;
        }
      }
      uint32 sig_state = temp_sig.GetState();
      printf("Signature index: %d, a PAdES signature. State after verifying: %s\r\n", i, TransformSignatureStateToString(sig_state).c_str());

      // Get PAdES level.
      Signature::PAdESLevel actual_level = temp_sig.GetPAdESLevel();
      printf("Signature index:%d, PAdES level:%s, %s\r\n", 
             i, TransformLevel2String(actual_level), 
             actual_level == expect_pades_level?"matching expected level." : "NOT match expected level.");
    }
  }
  if (false == has_cades_signature)
    printf("No PAdES signature in current PDF file.\r\n");
}

int main(int argc, char *argv[]) {
  if ((argc > 1 && String(argv[1]).Equal("--help")) || argc < 2) {
    PrintUsage();
    return 0;
  }

  int err_ret = 0;
  PadesCommand command;
  if (!AnalysisParameter(argc, argv, command)) {
    PrintUsage();
    return 1;
  }

  if (command.input_file == command.output_file) {
    cout << "Input and output path must be different." << endl;
    return 1;
  }

  if (!FileExists(command.input_file)) {
    cout << "Input PDF does not exist." << endl;
    return 1;
  }

  if (command.mode != e_ModeVerify) {
    if (command.cert_path.IsEmpty()) {
      cout << "Missing required parameter: --cert" << endl;
      return 1;
    }
    if (command.cert_password.IsEmpty()) {
      cout << "Missing required parameter: --cert-password" << endl;
      return 1;
    }
  }
  if (command.tsa_name.IsEmpty()) {
    command.tsa_name = FREE_ETSIRFC316TSA_SERVER_NAME;
  }
  if (command.tsa_url.IsEmpty()) {
    command.tsa_url = FREE_ETSIRFC316TSA_SERVER_URL;
  }

  if (command.mode != e_ModeVerify && !FileExists(command.cert_path)) {
    cout << "Certificate file does not exist." << endl;
    return 1;
  }

  EnsureDirectoryExists(ParentDirectory(command.output_file));

  SdkLibMgr sdk_lib_mgr;
  // Initialize library.
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  printf("Input file path: %s\r\n", (const char*)String::FromUnicode(command.input_file));
  try {
    bool ts_initialized = false;
    if (command.mode != e_ModeVerify) {
      TimeStampServerMgr::Initialize();
      ts_initialized = true;
      if (command.level >= Signature::e_PAdESLevelBT) {
        TimeStampServer timestamp_server = TimeStampServerMgr::AddServer(command.tsa_name, command.tsa_url, L"", L"");
        TimeStampServerMgr::SetDefaultServer(timestamp_server);
      }
      PAdESSign(command.input_file, command.output_file, command);
    }

    if (command.mode != e_ModeSign) {
      WString verify_pdf_path = (command.mode == e_ModeVerify) ? command.input_file : command.output_file;
      PAdESVerify(verify_pdf_path, command.level, command.has_sig_index ? command.sig_index : -1);
    }

    if (ts_initialized) {
      TimeStampServerMgr::Release();
    }
  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  } catch(...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}

