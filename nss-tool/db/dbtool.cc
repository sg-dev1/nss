/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dbtool.h"
#include "../common/argparse.h"
#include "../common/scoped_ptrs.h"

#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>

#include <cert.h>
#include <nss.h>
#include <certdb.h>

namespace nss_tool
{

static std::string
printFlags(unsigned int flags)
{
    std::stringstream ss;
    if (flags & CERTDB_VALID_CA) {
        if (!(flags & CERTDB_TRUSTED_CA) && !(flags & CERTDB_TRUSTED_CLIENT_CA)) {
            ss << "c";
        }
    }
    if (flags & CERTDB_TERMINAL_RECORD) {
        if (!(flags & CERTDB_TRUSTED)) {
            ss << "p";
        }
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

void
DBTool::usage()
{
    std::cout << "    nss db [--dbdir] list-certs\n";
}

bool
DBTool::run(std::vector<std::string> arguments)
{
    ArgParser parser;
    std::shared_ptr<ArgObject> dbDir = std::make_shared<ArgObject>("--dbDir", "Sets the path of the database directory");
    parser.add(dbDir);

    if (!parser.parse(arguments)) {
        // parsing error
        std::cout << "Parsing error!\n";
        return false;
    }

    std::string initDir(".");
    if (dbDir->isPresent()) {
        initDir = dbDir->getValue();
    }
    if (parser.getPositionalArgumentCount() != 1) {
        std::cout << "Positional Argument count wrong!\n";
        return false;
    }
    std::string subCommand = parser.getPositionalArgument(0);
    if (subCommand != "list-certs" && subCommand != "list-keys") {
        std::cout << "Unsupported subcommand given!\n";
        return false;
    }
    std::cout << "Using database directory: " << initDir << "\n";

    // init NSS
    const char *certPrefix = ""; // certutil -P option  --- can leave this empty
    SECStatus rv =
        NSS_Initialize(initDir.c_str(), certPrefix, certPrefix, "secmod.db", 0);
    if (rv != SECSuccess) {
        std::cout << "NSS init failed!\n";
        return false;
    }

    this->slot = ScopedPK11SlotInfo(PK11_GetInternalKeySlot());
    if (this->slot.get() == nullptr) {
        std::cout << "Error: Init PK11SlotInfo failed!\n";
        exit(1);
    }

    if (subCommand == "list-certs") {
        std::cout << "Listing certificates...\n";
        this->listCertificates();
    } else { // subCommand == "list-keys"
        std::cout << "Listing private keys...\n";
        this->listPrivateKeys();
    }

    return true;
}

DBTool::~DBTool()
{
    // shutdown nss
    if (NSS_Shutdown() != SECSuccess) {
        std::cout << "NSS Shutdown failed!\n";
    }
}

void
DBTool::listCertificates()
{
    ScopedCERTCertList list(PK11_ListCerts(PK11CertListAll, NULL));
    CERTCertListNode *node;

    for (node = CERT_LIST_HEAD(list); !CERT_LIST_END(node, list);
         node = CERT_LIST_NEXT(node)) {
        CERTCertTrust trust;
        CERTCertificate *cert;
        std::string trusts;
        std::string name;

        cert = node->cert;

        if (node->appData && ((char *)node->appData)[0]) {
            name = std::string((char *)node->appData);
        } else if (cert->nickname && cert->nickname[0]) {
            name = std::string(cert->nickname);
        } else if (cert->emailAddr && cert->emailAddr[0]) {
            name = std::string(cert->emailAddr);
        }

        if (CERT_GetCertTrust(cert, &trust) == SECSuccess) {
            std::stringstream ss;
            ss << printFlags(trust.sslFlags);
            ss << ",";
            ss << printFlags(trust.emailFlags);
            ss << ",";
            ss << printFlags(trust.objectSigningFlags);
            trusts = ss.str();
        } else {
            trusts = std::string(",,");
        }
        std::cout << std::setw(60) << std::left << name << " " << trusts << "\n";
    }
}

// TODO move this out in some utility module
static bool
itemIsPrintableASCII(ScopedSECItem const &item)
{
    std::string toTest((char *)item.get()->data);
    if (toTest.find_first_not_of(" !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~")) {
        return false;
    }
    return true;
}

static const char *const keyTypeName[] = {
    "null", "rsa", "dsa", "fortezza", "dh", "kea", "ec"
};

static std::string
stringToHex(const std::string &input)
{
    std::stringstream ss;
    for (std::string::size_type i = 0; i < input.length(); i++) {
        ss << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << (int)input[i];
    }

    return ss.str();
}

bool
DBTool::listPrivateKeys()
{
    ScopedSECKEYPrivateKeyList list(PK11_ListPrivateKeysInSlot(this->slot.get()));
    if (list.get() == nullptr) {
        std::cout << "Listing private keys failed!\n";
        return false;
    }
    SECKEYPrivateKeyListNode *node;
    int count = 0;

    for (node = PRIVKEY_LIST_HEAD(list); !PRIVKEY_LIST_END(node, list); node = PRIVKEY_LIST_NEXT(node)) {
        char *_keyname = PK11_GetPrivateKeyNickname(node->key);
        std::string keyName(_keyname == nullptr ? "(orphan)" : _keyname);

        if (keyName == "(orphan)") {
            ScopedCERTCertificate cert(PK11_GetCertFromPrivateKey(node->key));
            if (cert.get() != nullptr) {
                if (cert->nickname && cert->nickname[0]) {
                    keyName = std::string(cert->nickname);
                } else if (cert->emailAddr && cert->emailAddr[0]) {
                    keyName = std::string(cert->emailAddr);
                }
            }
            if (keyName == "(orphan)") {
                std::cout << "Warning: Current Key has no nickname!\n";
            }
        }

        SECKEYPrivateKey *key = node->key;
        ScopedSECItem secItem(PK11_GetLowLevelKeyIDForPrivateKey(key));
        std::string keyData;
        if (secItem.get() == nullptr) {
            std::cout << "Error: PK11_GetLowLevelKeyIDForPrivateKey failed!\n";
            continue;
        } else if (itemIsPrintableASCII(secItem)) {
            std::string tmp("'");
            keyData = tmp + std::string((char *)secItem->data) + tmp;
        } else {
            keyData = stringToHex(std::string((char *)secItem->data));
        }

        std::cout << "<" << count << ", name: " << keyName << "> " << keyTypeName[key->keyType]
                  << " " << keyData << "\n";

        count++;
    }

    return true;
}

} /* end namespace nss_tool */
