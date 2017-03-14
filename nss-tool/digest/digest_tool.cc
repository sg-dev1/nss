/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "digest_tool.h"
#include "argparse.h"

#include <iostream>

#include <hasht.h>
#include <secoid.h>

static SECOidData* HashTypeToOID(HASH_HashType hashtype) {
  SECOidTag hashtag;

  if (hashtype <= HASH_AlgNULL || hashtype >= HASH_AlgTOTAL) return nullptr;

  switch (hashtype) {
    case HASH_AlgMD2:
      hashtag = SEC_OID_MD2;
      break;
    case HASH_AlgMD5:
      hashtag = SEC_OID_MD5;
      break;
    case HASH_AlgSHA1:
      hashtag = SEC_OID_SHA1;
      break;
    case HASH_AlgSHA256:
      hashtag = SEC_OID_SHA256;
      break;
    case HASH_AlgSHA384:
      hashtag = SEC_OID_SHA384;
      break;
    case HASH_AlgSHA512:
      hashtag = SEC_OID_SHA512;
      break;
    case HASH_AlgSHA224:
      hashtag = SEC_OID_SHA224;
      break;
    default:
      std::cerr << "A new hash type has been added to HASH_HashType."
                << std::endl;
      std::cerr << "This program needs to be updated!" << std::endl;
      return nullptr;
  }

  return SECOID_FindOIDByTag(hashtag);
}

static SECOidData* HashNameToOID(const std::string& hashName) {
  int htypeInt;
  SECOidData* hashOID;

  for (htypeInt = HASH_AlgNULL + 1; htypeInt < HASH_AlgTOTAL; htypeInt++) {
    hashOID = HashTypeToOID(static_cast<HASH_HashType>(htypeInt));
    if (hashName == std::string(hashOID->desc)) break;
  }

  if (static_cast<HASH_HashType>(htypeInt) == HASH_AlgTOTAL) return nullptr;

  return hashOID;
}

bool DigestTool::Run(const std::vector<std::string>& arguments) {
  ArgParser parser(arguments);

  if (parser.GetPositionalArgumentCount() != 1) {
    Usage();
    return false;
  }

  // TODO handle --path  ---> common code from dbtool
  // TODO nss init

  const std::string hashName = parser.GetPositionalArgument(0);
  SECOidData* hashOID = HashNameToOID(hashName);
  if (hashOID == nullptr) {
    std::cerr << "Invalid digest type " << hashName << "." << std::endl;
    return false;
  }

  // TODO digest

  // TODO nss shutdown

  return true;
}

void DigestTool::Usage() {
  std::cerr << "Usage: nss digest [md5|sha1|sha256|sha512] [--path <directory>]"
            << std::endl;
}

bool DigestTool::Digest() {
  // #include <hasht.h>
  // unsigned char digest[HASH_LENGTH_MAX];
  // see hasht.h for supported digest types
  // TODO
  return true;
}
