/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dbtool.h"
#include "argparse.h"
#include "scoped_ptrs.h"

#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>

#include <cert.h>
#include <certdb.h>
#include <nss.h>
#include <prerror.h>
#include <prio.h>

static std::string PrintFlags(unsigned int flags) {
  std::stringstream ss;
  if ((flags & CERTDB_VALID_CA) && !(flags & CERTDB_TRUSTED_CA) &&
      !(flags & CERTDB_TRUSTED_CLIENT_CA)) {
    ss << "c";
  }
  if ((flags & CERTDB_TERMINAL_RECORD) && !(flags & CERTDB_TRUSTED)) {
    ss << "p";
  }
  if (flags & CERTDB_TRUSTED_CA) {
    ss << "C";
  }
  if (flags & CERTDB_TRUSTED_CLIENT_CA) {
    ss << "T";
  }
  if (flags & CERTDB_TRUSTED) {
    ss << "P";
  }
  if (flags & CERTDB_USER) {
    ss << "u";
  }
  if (flags & CERTDB_SEND_WARN) {
    ss << "w";
  }
  if (flags & CERTDB_INVISIBLE_CA) {
    ss << "I";
  }
  if (flags & CERTDB_GOVT_APPROVED_CA) {
    ss << "G";
  }
  return ss.str();
}

static std::vector<char> ReadFromIstream(std::istream &is) {
  std::vector<char> certData;
  while (is) {
    char buf[1024];
    is.read(buf, sizeof(buf));
    certData.insert(certData.end(), buf, buf + is.gcount());
  }

  return certData;
}

static bool InitSlotPassword(void) {
  ScopedPK11SlotInfo slot = ScopedPK11SlotInfo(PK11_GetInternalKeySlot());
  if (slot.get() == nullptr) {
    std::cerr << "Error: Init PK11SlotInfo failed!" << std::endl;
    return false;
  }

  std::cout << "Enter a password which will be used to encrypt your keys."
            << std::endl
            << std::endl;
  std::string pw, pwComp;

  while (true) {
    std::cout << "Enter new password: " << std::endl;
    std::getline(std::cin, pw);  // TODO disable echoing of password to stdout
    std::cout << "Re-enter password: " << std::endl;
    std::getline(std::cin, pwComp);  // TODO disable echoing of pw to stdout

    if (pw == pwComp) {
      break;
    }

    std::cerr << "Passwords do not match. Try again." << std::endl;
  }

  SECStatus rv = PK11_InitPin(slot.get(), nullptr, pw.c_str());
  if (rv != SECSuccess) {
    std::cerr << "Init db password failed." << std::endl;
    return false;
  }

  return true;
}

enum PwDataType { PW_NONE = 0, PW_FROMFILE = 1, PW_PLAINTEXT = 2 };
typedef struct {
  PwDataType source;
  char *data;
} PwData;

static char *GetModulePassword(PK11SlotInfo *slot, int retry, void *arg) {
  PwData *pwData = reinterpret_cast<PwData *>(arg);

  if (pwData == nullptr) {
    return nullptr;
  }

  if (retry > 0) {
    std::cerr << "Incorrect password/PIN entered." << std::endl;
    return nullptr;
  }

  switch (pwData->source) {
    case PW_NONE:
    case PW_FROMFILE:
      std::cerr << "Password input method not supported." << std::endl;
      return nullptr;
    case PW_PLAINTEXT:
      return PL_strdup(pwData->data);
    default:
      break;
  }

  std::cerr << "Password check failed:  No password found." << std::endl;
  return nullptr;
}

static const char *const keyTypeName[] = {"null", "rsa", "dsa", "fortezza",
                                          "dh",   "kea", "ec"};

static std::string StringToHex(const std::string &input) {
  std::stringstream ss("0x");
  for (std::string::size_type i = 0; i < input.length(); i++) {
    ss << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
       << (int)input[i];
  }

  return ss.str();
}

void DBTool::Usage() {
  std::cerr << "Usage: nss db [--path <directory>]" << std::endl;
  std::cerr << "  --create" << std::endl;
  std::cerr << "  --list-certs" << std::endl;
  std::cerr << "  --import-cert [<path>] --name <name> [--trusts <trusts>]"
            << std::endl;
  std::cerr << "  --list-keys" << std::endl;
}

bool DBTool::Run(const std::vector<std::string> &arguments) {
  ArgParser parser(arguments);

  // xor to assert that exactly one command is given
  if (((parser.Has("--create") ? 1 : 0) + (parser.Has("--list-certs") ? 1 : 0) +
       (parser.Has("--import-cert") ? 1 : 0) +
       (parser.Has("--list-keys") ? 1 : 0)) != 1) {
    return false;
  }

  PRAccessHow how = PR_ACCESS_READ_OK;
  bool readOnly = true;
  if (parser.Has("--create") || parser.Has("--import-cert")) {
    how = PR_ACCESS_WRITE_OK;
    readOnly = false;
  }

  std::string initDir(".");
  if (parser.Has("--path")) {
    initDir = parser.Get("--path");
  }
  if (PR_Access(initDir.c_str(), how) != PR_SUCCESS) {
    std::cerr << "Directory '" << initDir
              << "' does not exist or you don't have permissions!" << std::endl;
    return false;
  }

  std::cout << "Using database directory: " << initDir << std::endl
            << std::endl;

  bool dbFilesExist = PathHasDBFiles(initDir);
  if (parser.Has("--create") && dbFilesExist) {
    std::cerr << "Trying to create database files in a directory where they "
                 "already exists. Delete the db files before creating new ones."
              << std::endl;
    return false;
  }
  if (!parser.Has("--create") && !dbFilesExist) {
    std::cerr << "No db files found." << std::endl;
    std::cerr << "Create them using 'nss db --create [--path /foo/bar]' before "
                 "continuing."
              << std::endl;
    return false;
  }

  // init NSS
  const char *certPrefix = "";  // certutil -P option  --- can leave this empty
  SECStatus rv = NSS_Initialize(initDir.c_str(), certPrefix, certPrefix,
                                "secmod.db", readOnly ? NSS_INIT_READONLY : 0);
  if (rv != SECSuccess) {
    std::cerr << "NSS init failed!" << std::endl;
    return false;
  }

  bool ret = true;
  if (parser.Has("--list-certs")) {
    ListCertificates();
  } else if (parser.Has("--import-cert")) {
    ret = ImportCertificate(parser);
  } else if (parser.Has("--create")) {
    ret = InitSlotPassword();
    if (ret) {
      std::cout << "DB files created successfully." << std::endl;
    }
  } else if (parser.Has("--list-keys")) {
    ret = ListKeys();
  }

  // shutdown nss
  if (NSS_Shutdown() != SECSuccess) {
    std::cerr << "NSS Shutdown failed!" << std::endl;
    return false;
  }

  return ret;
}

bool DBTool::PathHasDBFiles(std::string path) {
  std::regex certDBPattern("cert.*\\.db");
  std::regex keyDBPattern("key.*\\.db");

  DIR *dir;
  if (!(dir = opendir(path.c_str()))) {
    std::cerr << "Directory " << path << " could not be accessed!" << std::endl;
    return false;
  }

  struct dirent *ent;
  bool dbFileExists = false;
  while ((ent = readdir(dir))) {
    if (std::regex_match(ent->d_name, certDBPattern) ||
        std::regex_match(ent->d_name, keyDBPattern) ||
        "secmod.db" == std::string(ent->d_name)) {
      dbFileExists = true;
      break;
    }
  }

  closedir(dir);
  return dbFileExists;
}

void DBTool::ListCertificates() {
  ScopedCERTCertList list(PK11_ListCerts(PK11CertListAll, nullptr));
  CERTCertListNode *node;

  std::cout << std::setw(60) << std::left << "Certificate Nickname"
            << " "
            << "Trust Attributes" << std::endl;
  std::cout << std::setw(60) << std::left << ""
            << " "
            << "SSL,S/MIME,JAR/XPI" << std::endl
            << std::endl;

  for (node = CERT_LIST_HEAD(list); !CERT_LIST_END(node, list);
       node = CERT_LIST_NEXT(node)) {
    CERTCertificate *cert = node->cert;

    std::string name("(unknown)");
    char *appData = static_cast<char *>(node->appData);
    if (appData && strlen(appData) > 0) {
      name = appData;
    } else if (cert->nickname && strlen(cert->nickname) > 0) {
      name = cert->nickname;
    } else if (cert->emailAddr && strlen(cert->emailAddr) > 0) {
      name = cert->emailAddr;
    }

    CERTCertTrust trust;
    std::string trusts;
    if (CERT_GetCertTrust(cert, &trust) == SECSuccess) {
      std::stringstream ss;
      ss << PrintFlags(trust.sslFlags);
      ss << ",";
      ss << PrintFlags(trust.emailFlags);
      ss << ",";
      ss << PrintFlags(trust.objectSigningFlags);
      trusts = ss.str();
    } else {
      trusts = ",,";
    }
    std::cout << std::setw(60) << std::left << name << " " << trusts
              << std::endl;
  }
}

bool DBTool::ImportCertificate(const ArgParser &parser) {
  if (!parser.Has("--name")) {
    std::cerr << "A name (--name) is required to import a certificate."
              << std::endl;
    return false;
  }

  std::string derFilePath = parser.Get("--import-cert");
  std::string certName = parser.Get("--name");
  std::string trustString("TCu,Cu,Tu");
  if (parser.Has("--trusts")) {
    trustString = parser.Get("--trusts");
  }

  CERTCertTrust trust;
  SECStatus rv = CERT_DecodeTrustString(&trust, trustString.c_str());
  if (rv != SECSuccess) {
    std::cerr << "Cannot decode trust string!" << std::endl;
    return false;
  }

  ScopedPK11SlotInfo slot = ScopedPK11SlotInfo(PK11_GetInternalKeySlot());
  if (slot.get() == nullptr) {
    std::cerr << "Error: Init PK11SlotInfo failed!" << std::endl;
    return false;
  }

  std::vector<char> certData;
  if (derFilePath.empty()) {
    std::cout << "No Certificate file path given, using stdin." << std::endl;
    certData = ReadFromIstream(std::cin);
  } else {
    std::ifstream is(derFilePath, std::ifstream::binary);
    if (!is.good()) {
      std::cerr << "IO Error when opening " << derFilePath << std::endl;
      std::cerr
          << "Certificate file does not exist or you don't have permissions."
          << std::endl;
      return false;
    }
    certData = ReadFromIstream(is);
  }

  ScopedCERTCertificate cert(
      CERT_DecodeCertFromPackage(certData.data(), certData.size()));
  if (cert.get() == nullptr) {
    std::cerr << "Error: Could not decode certificate!" << std::endl;
    return false;
  }

  rv = PK11_ImportCert(slot.get(), cert.get(), CK_INVALID_HANDLE,
                       certName.c_str(), PR_FALSE);
  if (rv != SECSuccess) {
    // TODO handle authentication -> PK11_Authenticate (see certutil.c line
    // 134)
    std::cerr << "Error: Could not add certificate to database!" << std::endl;
    return false;
  }

  rv = CERT_ChangeCertTrust(CERT_GetDefaultCertDB(), cert.get(), &trust);
  if (rv != SECSuccess) {
    std::cerr << "Cannot change cert's trust" << std::endl;
    return false;
  }

  std::cout << "Certificate import was successful!" << std::endl;
  // TODO show information about imported certificate
  return true;
}

bool DBTool::ListKeys() {
  ScopedPK11SlotInfo slot = ScopedPK11SlotInfo(PK11_GetInternalKeySlot());
  if (slot.get() == nullptr) {
    std::cerr << "Error: Init PK11SlotInfo failed!" << std::endl;
    return false;
  }

  PK11_SetPasswordFunc(&GetModulePassword);
  if (PK11_NeedLogin(slot.get())) {
    std::string pw;
    std::cout << "Enter your password: " << std::endl;
    std::getline(std::cin, pw);  // TODO disable echoing of password to stdout
    PwData pwData = {PW_PLAINTEXT, const_cast<char *>(pw.c_str())};
    SECStatus rv = PK11_Authenticate(slot.get(), true /*loadCerts*/, &pwData);
    if (rv != SECSuccess) {
      std::cerr << "Could not authenticate to token "
                << PK11_GetTokenName(slot.get()) << ". Failed with code "
                << PR_GetError() << std::endl;
      return false;
    }
  }

  ScopedSECKEYPrivateKeyList list(PK11_ListPrivateKeysInSlot(slot.get()));
  if (list.get() == nullptr) {
    std::cerr << "Listing private keys failed!" << std::endl;
    return false;
  }

  std::cout << std::setw(20) << std::left << "<key#, key name>" << std::setw(20)
            << std::left << " key type "
            << " key id " << std::endl;

  SECKEYPrivateKeyListNode *node;
  int count = 0;
  for (node = PRIVKEY_LIST_HEAD(list.get());
       !PRIVKEY_LIST_END(node, list.get()); node = PRIVKEY_LIST_NEXT(node)) {
    char *keyNameRaw = PK11_GetPrivateKeyNickname(node->key);
    std::string keyName(keyNameRaw == nullptr ? "" : keyNameRaw);

    if (keyName.empty()) {
      ScopedCERTCertificate cert(PK11_GetCertFromPrivateKey(node->key));
      if (cert.get() != nullptr) {
        if (cert->nickname && strlen(cert->nickname) > 0) {
          keyName = cert->nickname;
        } else if (cert->emailAddr && strlen(cert->emailAddr) > 0) {
          keyName = cert->emailAddr;
        }
      }
      if (keyName.empty()) {
        keyName = "(none)";  // default value
      }
    }

    SECKEYPrivateKey *key = node->key;
    ScopedSECItem keyIDItem(PK11_GetLowLevelKeyIDForPrivateKey(key));
    if (keyIDItem.get() == nullptr) {
      std::cerr << "Error: PK11_GetLowLevelKeyIDForPrivateKey failed!"
                << std::endl;
      continue;
    }

    std::string keyID = StringToHex(
        std::string(reinterpret_cast<char *>(keyIDItem->data), keyIDItem->len));

    std::cout << std::setw(20) << std::left << "<" << count++
              << ", name: " << keyName << "> " << std::setw(20) << std::left
              << keyTypeName[key->keyType] << " " << keyID << std::endl;
  }

  return true;
}
