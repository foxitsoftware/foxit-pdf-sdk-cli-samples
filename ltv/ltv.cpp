// Copyright (C) 2003-2026, Foxit Software Inc..
// All Rights Reserved.
//
// http://www.foxitsoftware.com
//
// The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
// You cannot distribute any part of Foxit PDF SDK to any third party or general public,
// unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
//
// This file contains an example to demonstrate how to use Foxit PDF SDK to do LTV verification (using default callback) in PDF document.

// Include header files.
#include <iostream>
#include <time.h>
#include <ctime>

// Include Foxit SDK header files.
#include "../../../include/pdf/annots/fs_annot.h"
#include "../../../include/common/fs_image.h"
#include "../../../include/pdf/fs_pdfdoc.h"
#include "../../../include/pdf/fs_pdfpage.h"
#include "../../../include/pdf/fs_signature.h"
#include "../../../include/pdf/fs_ltvverifier.h"

using namespace std;
using namespace foxit;
using namespace foxit::common;
using foxit::common::Library;
using namespace pdf;
using namespace objects;
using namespace file;

struct CliOptions {
  WString input_file;
  WString output_file;
  WString cert_path;
  WString cert_password;
  Signature::DigestAlgorithm digest_algorithm;
  WString signer;
  WString contact_info;
  WString dn;
  WString location;
  WString reason;
  bool show_help;

  CliOptions() : digest_algorithm(Signature::e_DigestSHA256), show_help(false) {}
};

void PrintUsage() {
  cout << "Usage:" << endl
       << "ltv -i <input pdf path> -o <output pdf path> --cert <cert path> --cert-password <password> [--digest <sha1|sha256|sha384|sha512>]" << endl
       << "    [--signer <name>] [--contact <contact info>] [--dn <DN string>] [--location <location>] [--reason <reason>]" << endl
       << endl
       << "Required:" << endl
       << "  -i <path>                Input PDF path" << endl
       << "  -o <path>                Output PDF path" << endl
       << "  --cert <path>            Certificate file path (e.g. .pfx/.p12)" << endl
       << "  --cert-password <pwd>    Certificate password" << endl
       << endl
       << "Optional:" << endl
       << "  --digest <algorithm>     Digest algorithm: sha1, sha256 (default), sha384, sha512" << endl
       << "  --signer <name>          Signer name (default: \"Foxit PDF SDK\")" << endl
       << "  --contact <info>         Contact info (default: \"support@foxitsoftware.com\")" << endl
       << "  --dn <string>            DN string (default: \"CN=CN,MAIL=MAIL@MAIL.COM\")" << endl
       << "  --location <string>      Location (default: \"Fuzhou, China\")" << endl
       << "  --reason <string>        Reason for signing (default: auto-generated)" << endl
       << "  --help                   Show this help message" << endl;
}

bool ParseDigestAlgorithm(const String& digest, Signature::DigestAlgorithm& out) {
  if (digest.Equal("sha1")) { out = Signature::e_DigestSHA1; return true; }
  if (digest.Equal("sha256")) { out = Signature::e_DigestSHA256; return true; }
  if (digest.Equal("sha384")) { out = Signature::e_DigestSHA384; return true; }
  if (digest.Equal("sha512")) { out = Signature::e_DigestSHA512; return true; }
  return false;
}

bool ParseArgs(int argc, char* argv[], CliOptions& options) {
  for (int i = 1; i < argc; ++i) {
    String argv_key = String(argv[i]);
    if (argv_key.Equal("--help")) {
      options.show_help = true;
      return true;
    }

    if (argc <= i + 1) {
      PrintUsage();
      return false;
    }

    String argv_value = String(argv[++i]);
    if (argv_key.Equal("-i")) options.input_file = WString::FromUTF8(argv_value);
    else if (argv_key.Equal("-o")) options.output_file = WString::FromUTF8(argv_value);
    else if (argv_key.Equal("--cert")) options.cert_path = WString::FromUTF8(argv_value);
    else if (argv_key.Equal("--cert-password")) options.cert_password = WString::FromUTF8(argv_value);
    else if (argv_key.Equal("--digest")) {
      if (!ParseDigestAlgorithm(argv_value, options.digest_algorithm)) {
        cout << "Invalid value for --digest: " << argv_value << ". Expected: sha1, sha256, sha384, sha512." << endl;
        return false;
      }
    }
    else if (argv_key.Equal("--signer")) options.signer = WString::FromUTF8(argv_value);
    else if (argv_key.Equal("--contact")) options.contact_info = WString::FromUTF8(argv_value);
    else if (argv_key.Equal("--dn")) options.dn = WString::FromUTF8(argv_value);
    else if (argv_key.Equal("--location")) options.location = WString::FromUTF8(argv_value);
    else if (argv_key.Equal("--reason")) options.reason = WString::FromUTF8(argv_value);
    else {
      PrintUsage();
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
  if (options.cert_path.IsEmpty()) {
    cout << "Missing required parameter: --cert" << endl;
    return false;
  }
  if (options.cert_password.IsEmpty()) {
    cout << "Missing required parameter: --cert-password" << endl;
    return false;
  }
  return true;
}

// sn and key information from Foxit PDF SDK's key files are used to initialize Foxit PDF SDK library. 
static const char* sn = "";
static const char* key = "";

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

#define FREE_ETSIRFC316TSA_SERVER_NAME L"FreeTSAServer"
#define FREE_ETSIRFC316TSA_SERVER_URL L"http://ca.signfiles.com/TSAServer.aspx"

#define PKCS7_SIGNATURE_FILTER "Adobe.PPKLite"
#define PKCS7_SIGNATURE_SUBFILTER "adbe.pkcs7.detached"


FILE* OpenFileWrapper(const char* file_name, const char* file_mode) {
  FILE* ret_file = NULL;
#if defined(_WIN32) || defined(_WIN64)
  fopen_s(&ret_file, (const char*)file_name, (const char*)file_mode);
#else
  ret_file = fopen((const char*)file_name, (const char*)file_mode);
#endif
  return ret_file;
}

string TransformSignatureStateToString(uint32 sig_state) {
  string state_str;
  if (sig_state & Signature::e_StateUnknown)
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

void PKCS7Signature(const CliOptions& options, const WString& signed_pdf_path) {
  PDFDoc pdf_doc(options.input_file);
  pdf_doc.StartLoad();
  PDFPage pdf_page = pdf_doc.GetPage(0);
  float page_width = pdf_page.GetWidth();
  float page_height = pdf_page.GetHeight();
  RectF new_sig_rect((page_width / 2 - 50.0f), (page_height / 2 - 50.0f), (page_width / 2 + 50.0f), (page_height / 2 + 50.0f));
  Signature new_signature = pdf_page.AddSignature(new_sig_rect);
  new_signature.SetFilter(PKCS7_SIGNATURE_FILTER);
  new_signature.SetSubFilter(PKCS7_SIGNATURE_SUBFILTER);

  WString signer = options.signer.IsEmpty() ? L"Foxit PDF SDK" : options.signer;
  WString contact_info = options.contact_info.IsEmpty() ? L"support@foxitsoftware.com" : options.contact_info;
  WString dn = options.dn.IsEmpty() ? L"CN=CN,MAIL=MAIL@MAIL.COM" : options.dn;
  WString location = options.location.IsEmpty() ? L"Fuzhou, China" : options.location;
  new_signature.SetKeyValue(Signature::e_KeyNameSigner, signer);
  new_signature.SetKeyValue(Signature::e_KeyNameContactInfo, contact_info);
  new_signature.SetKeyValue(Signature::e_KeyNameDN, dn);
  new_signature.SetKeyValue(Signature::e_KeyNameLocation, location);
  String new_value;
  if (options.reason.IsEmpty()) {
    new_value.Format("As a sample for subfilter \"%s\" ", PKCS7_SIGNATURE_SUBFILTER);
  } else {
    new_value = String((const char*)String::FromUnicode(options.reason));
  }
  new_signature.SetKeyValue(Signature::e_KeyNameReason, (const wchar_t*)WString::FromLocal(new_value));
  new_signature.SetKeyValue(Signature::e_KeyNameText, (const wchar_t*)WString::FromLocal(new_value));
  DateTime sign_time = DateTime::GetLocalTime();
  new_signature.SetSignTime(sign_time);
  // Set appearance flags to decide which content would be used in appearance.
  uint32 ap_flags = Signature::e_APFlagLabel | Signature::e_APFlagSigner |
    Signature::e_APFlagReason | Signature::e_APFlagDN |
    Signature::e_APFlagLocation | Signature::e_APFlagText |
    Signature::e_APFlagSigningTime;
  new_signature.SetAppearanceFlags(ap_flags);

  Progressive sign_progressive = new_signature.StartSign(options.cert_path, options.cert_password, options.digest_algorithm, signed_pdf_path);
  if (sign_progressive.GetRateOfProgress() != 100) {
    if (Progressive::e_Finished != sign_progressive.Continue()) {
      printf("[Failed] Fail to sign the new CAdES signature.\r\n");
      return;
    }
  }
}

// Here, the implementation of TrustedCertStoreCallback is very simple : 
// trust all input certificate(s) when this callback function is triggered during LTV verification.
// User can improve the implementation of the callback class TrustedCertStoreCallback or choose not to use TrustedCertStoreCallback.
class MyTrustedCertStoreCallback : public TrustedCertStoreCallback {
public:
  MyTrustedCertStoreCallback() {}
  ~MyTrustedCertStoreCallback() {}

  virtual bool IsCertTrusted(const String& cert) {
    return true;
  }
  
  virtual bool IsCertTrustedRoot(const String& cert) {
    return true;
  }
};

void UseLTVVerifier(const PDFDoc& pdf_doc, bool is_to_add_dss) {
  // Here use default RevocationCallback, so no need to call LTVVerifier::SetRevocationCallback
  LTVVerifier ltv_verifier(pdf_doc, true, true, false, LTVVerifier::e_SignatureCreationTime);

  // Use implemented TrustedCertStoreCallback to trust some cerificates during LTV verification.
  // Here, the implementation of TrustedCertStoreCallback is very simple : 
  // trust all input certificate(s) when this callback function is triggered during LTV verification.
  // User can improve the implementation of the callback class TrustedCertStoreCallback or choose not to use TrustedCertStoreCallback.
  MyTrustedCertStoreCallback my_callback;
  ltv_verifier.SetTrustedCertStoreCallback(&my_callback);

  ltv_verifier.SetVerifyMode(LTVVerifier::e_VerifyModeAcrobat);

  SignatureVerifyResultArray sig_verify_result_array = ltv_verifier.Verify();
  for (size_t i = 0; i < sig_verify_result_array.GetSize(); i++) {
    SignatureVerifyResult sig_verify_result = sig_verify_result_array.GetAt(i);
    String signature_name = sig_verify_result.GetSignatureName();
    uint32 sig_state = sig_verify_result.GetSignatureState();
    SignatureVerifyResult::LTVState ltv_state = sig_verify_result.GetLTVState();
    string ltv_state_str;
    switch (ltv_state) {
      case SignatureVerifyResult::e_LTVStateInactive:
        ltv_state_str = "inactive";
        break;
      case SignatureVerifyResult::e_LTVStateEnable:
        ltv_state_str = "enabled";
        break;
      case SignatureVerifyResult::e_LTVStateNotEnable:
        ltv_state_str = "not enabled";
        break;
    }
    printf("Signature name:%s, signature state: %s, LTV state: %s\r\n",
           signature_name.GetBuffer(signature_name.GetLength()),
           TransformSignatureStateToString(sig_state).c_str(),
           ltv_state_str.c_str());
    signature_name.ReleaseBuffer();
  }

  if (is_to_add_dss) {
    for (size_t i = 0; i < sig_verify_result_array.GetSize(); i++) {
      if (sig_verify_result_array.GetAt(i).GetSignatureState() & Signature::e_StateVerifyValid)
        ltv_verifier.AddDSS(sig_verify_result_array.GetAt(i));
    }
  }
}

void DoLTV(const WString& signed_pdf_path, const WString& saved_ltv_pdf_path) {
  // Use default SignatureCallback for signing a time stamp signature with filter "Adobe.PPKLite" and subfilter "ETSI.RFC3161",
  // so no need to register a custom signature callback.

  TimeStampServerMgr::Initialize();
  TimeStampServer timestamp_server = TimeStampServerMgr::AddServer(FREE_ETSIRFC316TSA_SERVER_NAME, FREE_ETSIRFC316TSA_SERVER_URL, L"", L"");
  TimeStampServerMgr::SetDefaultServer(timestamp_server);

  PDFDoc pdf_doc(signed_pdf_path);
  pdf_doc.StartLoad();
  // Add DSS
  printf("== Before Add DSS ==\r\n");
  UseLTVVerifier(pdf_doc, true);

  // Add DTS
  PDFPage pdf_page = pdf_doc.GetPage(0);
  // The new time stamp signature will have default filter name "Adobe.PPKLite" and default subfilter name "ETSI.RFC3161".
  Signature timestamp_signature = pdf_page.AddSignature(RectF(), L"", Signature::e_SignatureTypeTimeStamp);
  Progressive sign_progressive = timestamp_signature.StartSign(L"", L"", Signature::e_DigestSHA256, saved_ltv_pdf_path);
  if (sign_progressive.GetRateOfProgress() != 100)
    sign_progressive.Continue();

  // Check saved file.
  PDFDoc check_pdf_doc(saved_ltv_pdf_path);
  check_pdf_doc.StartLoad();
  // Just LTV veify.
  printf("== After Add DSS ==\r\n");
  UseLTVVerifier(check_pdf_doc, false);

  TimeStampServerMgr::Release();
}

int main(int argc, char *argv[]) {
  CliOptions options;
  if (!ParseArgs(argc, argv, options)) return 1;
  if (options.show_help) {
    PrintUsage();
    return 0;
  }
  if (!ValidateArgs(options)) return 1;

  int err_ret = 0;

  WString signed_pdf_path = options.output_file;
  if (signed_pdf_path.GetLength() > 4) {
    signed_pdf_path = signed_pdf_path.Left(signed_pdf_path.GetLength() - 4) + L"_signed.pdf";
  } else {
    signed_pdf_path += L"_signed.pdf";
  }

  SdkLibMgr sdk_lib_mgr;
  // Initialize library.
  ErrorCode error_code = sdk_lib_mgr.Initialize();
  if (error_code != foxit::e_ErrSuccess) {
    return 1;
  }

  printf("Input file path: %s\r\n", (const char*)String::FromUnicode(options.input_file));
  printf("Output file path: %s\r\n", (const char*)String::FromUnicode(options.output_file));
  try {
    // Add a PKCS7 signature and sign it.
    PKCS7Signature(options, signed_pdf_path);

    // Do LTV
    DoLTV(signed_pdf_path, options.output_file);

  } catch (const Exception& e) {
    cout << e.GetMessage() << endl;
    err_ret = 1;
  } catch(...) {
    cout << "Unknown Exception" << endl;
    err_ret = 1;
  }

  return err_ret;
}

