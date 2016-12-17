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
#include <fstream>

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
    std::cout << "    nss db [--dbdir] [--derFile] [--certName] list-certs|import-cert\n";
}

bool
DBTool::run(std::vector<std::string> arguments)
{
    ArgParser parser;
    std::shared_ptr<ArgObject> dbDirObj = std::make_shared<ArgObject>("--dbDir", "Sets the path of the database directory");
    std::shared_ptr<ArgObject> derFileObj = std::make_shared<ArgObject>("--derFile", "Sets the path of a DER file to import");
    std::shared_ptr<ArgObject> certNameObj = std::make_shared<ArgObject>("--certName", "Sets the name of an imported certificate");
    parser.add(dbDirObj);
    parser.add(derFileObj);
    parser.add(certNameObj);

    if (!parser.parse(arguments)) {
        // parsing error
        std::cout << "Parsing error!\n";
        return false;
    }

    std::string initDir(".");
    if (dbDirObj->isPresent()) {
        initDir = dbDirObj->getValue();
    }
    std::string derFilePath;
    if (derFileObj->isPresent()) {
        derFilePath = std::string(derFileObj->getValue());
    }
    std::string certName("(no name)");
    if (certNameObj->isPresent()) {
        certName = std::string(certNameObj->getValue());
    }
    if (parser.getPositionalArgumentCount() != 1) {
        std::cout << "Positional Argument count wrong!\n";
        return false;
    }
    std::string subCommand = parser.getPositionalArgument(0);
    if (subCommand != "list-certs" && subCommand != "import-cert") {
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
    } else { // subCommand == "import-cert"
        std::cout << "Importing a certificate...\n";
        this->importCertificate(derFilePath, certName);
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
            if (trusts == ",,") {
                trusts = std::string("(no trusts found)");
            }
        } else {
            trusts = std::string("(no trusts found)");
        }
        std::cout << std::setw(60) << std::left << name << " " << trusts << "\n";
    }
}

void
DBTool::importCertificate(std::string derFilePath, std::string certName)
{
    SECItem certDER;

    if (derFilePath.empty()) {
        // read from stdin
        // implementation taken from certutil -> basicutil.c: secu_StdinToItem
        // TODO improve
        unsigned char buf[1000];
        PRInt32 numBytes;

        certDER.len = 0;
        certDER.data = nullptr;
        while (true) {
            numBytes = PR_Read(PR_STDIN, buf, sizeof(buf));
            if (numBytes < 0) {
                std::cout << "Error reading from stdin!\n";
                exit(-1);
            }

            if (numBytes == 0) {
                break;
            }

            if (certDER.data != nullptr) {
                unsigned char *p = certDER.data;
                certDER.data = (unsigned char *)PORT_Realloc(p, certDER.len + numBytes);
                if (!certDER.data) {
                    PORT_Free(p);
                }
            } else {
                certDER.data = (unsigned char *)PORT_Alloc(numBytes);
            }

            if (!certDER.data) {
                std::cout << "Error reading from stdin!\n";
                exit(-1);
            }
            PORT_Memcpy(certDER.data + certDER.len, buf, numBytes);
            certDER.len += numBytes;
        }
    } else {
        std::ifstream file(derFilePath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cout << "Error: Could not open DER file " << derFilePath << "!\n";
            exit(-1);
        }

        std::streampos size = file.tellg();
        char *memblock = new char[size];
        file.seekg(0, std::ios::beg);
        file.read(memblock, size);
        file.close();

        certDER.data = (unsigned char *)memblock;
        certDER.len = size;
    }
    //std::cout << "File read finished!\n";

    ScopedCERTCertificate cert(CERT_DecodeCertFromPackage((char *)certDER.data, certDER.len));
    if (cert.get() == nullptr) {
        std::cout << "Error: Could not decode certificate!\n";
        exit(-1);
    }

    // TODO handle Cert Trust

    SECStatus rv = PK11_ImportCert(this->slot.get(), cert.get(), CK_INVALID_HANDLE, certName.c_str(), PR_FALSE);
    if (rv != SECSuccess) {
        // TODO handle authentication -> PK11_Authenticate (see certutil.c line 134)
        std::cout << "Error: Could not add certificate to database!\n";
        exit(-1);
    }
}

} /* end namespace nss_tool */
