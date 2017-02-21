/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "keytool.h"
#include "scoped_ptrs.h"

#include <iostream>

#include <nss.h>
#include <pk11pub.h>
#include <prerror.h>

void KeyTool::Usage() {
  std::cerr << "Usage: nss key [--path <directory>]" << std::endl;
  std::cerr << "  --genKeyPair [--algo <rsa|dsa|ec>]" << std::endl;
}

bool KeyTool::Run(const std::vector<std::string>& arguments) {
  ArgParser parser(arguments);

  if (!parser.Has("--genKeyPair")) {
    return false;
  }

  PRAccessHow how = PR_ACCESS_WRITE_OK;
  bool readOnly = false;

  std::string initDir(".");
  if (parser.Has("--path")) {
    initDir = parser.Get("--path");
  }
  if (PR_Access(initDir.c_str(), how) != PR_SUCCESS) {
    std::cerr << "Directory '" << initDir
              << "' does not exist or you don't have permissions!" << std::endl;
    return false;
  }

  // TODO here we must check if the dbFiles exist (see dbtool.PathHasDBFiles)
  //   so put PathHasDBFiles f.e. into util module

  // init NSS
  const char* certPrefix = "";  // certutil -P option  --- can leave this empty
  SECStatus rv = NSS_Initialize(initDir.c_str(), certPrefix, certPrefix,
                                "secmod.db", readOnly ? NSS_INIT_READONLY : 0);
  if (rv != SECSuccess) {
    std::cerr << "NSS init failed!" << std::endl;
    return false;
  }

  bool ret = GeneratePrivPubKeyPair(parser);

  // shutdown nss
  if (NSS_Shutdown() != SECSuccess) {
    std::cerr << "NSS Shutdown failed!" << std::endl;
    return false;
  }

  return ret;
}

bool KeyTool::GeneratePrivPubKeyPair(const ArgParser& parser) {
  // TODO arg checking

  ScopedPK11SlotInfo slot(PK11_GetInternalKeySlot());
  if (!slot.get()) {
    std::cerr << "Error: Init PK11SlotInfo failed!\n";
    return false;
  }

  // TODO authenticate

  unsigned char randBuf[2048];
  SECStatus rv = PK11_GenerateRandom(randBuf, sizeof(randBuf));
  if (rv != SECSuccess) {
    std::cerr << "Error: PK11_GenerateRandom failed!" << std::endl;
    return false;
  }
  rv = PK11_RandomUpdate(randBuf, sizeof(randBuf));
  if (rv != SECSuccess) {
    std::cerr << "Error: PK11_RandomUpdate failed!" << std::endl;
    return false;
  }
  memset(randBuf, 0, sizeof(randBuf));

  CK_MECHANISM_TYPE mechanism;
  void* params;

  // TODO support of more than RSA algorithm (DSA + EC)
  PK11RSAGenParams rsaParams;
  // TODO these params should come from commandline
  rsaParams.keySizeInBits = 1024;
  rsaParams.pe = 65537;
  mechanism = CKM_RSA_PKCS_KEY_PAIR_GEN;
  params = &rsaParams;

  // XXX currently this will fail with SEC_ERROR_TOKEN_NOT_LOGGED_IN
  //   code: -8037 since we need to login first
  SECKEYPublicKey* pubKeyRaw;
  ScopedSECKEYPrivateKey privKey(PK11_GenerateKeyPair(
      slot.get(), mechanism, params, &pubKeyRaw, true, true, nullptr));
  if (!privKey.get()) {
    std::cerr << "Generate key pair failed with code " << PR_GetError()
              << std::endl;
    return false;
  }
  ScopedSECKEYPublicKey pubKey(pubKeyRaw);

  std::string nickName;  // TODO get from arg
  rv = PK11_SetPrivateKeyNickname(privKey.get(), nickName.c_str());
  if (rv != SECSuccess) {
    std::cerr << "Set nickname for private key failed." << std::endl;
    return false;
  }

  // TODO do something with generated keys: e.g. print, write to file etc.
  std::cout << "Generate key pair was successful." << std::endl;

  return true;
}
