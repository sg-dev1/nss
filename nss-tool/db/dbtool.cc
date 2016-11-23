/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dbtool.h"
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include "../common/argparse.h"
#include "../common/scoped_ptrs.h"

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

DBTool::DBTool(std::vector<std::string> arguments)
{
    ArgParser parser;
    std::shared_ptr<ArgObject> dbDir = std::make_shared<ArgObject>("--dbDir", "Sets the path of the database directory");
    parser.add(dbDir);

    if (!parser.parse(arguments)) {
        // parsing error
        std::cout << "Parsing error!\n";
        this->error = true;
        return;
    }

    std::string initDir(".");
    if (dbDir->isPresent()) {
        initDir = dbDir->getValue();
    }
    if (parser.getPositionalArgumentCount() != 1) {
        std::cout << "Positional Argument count wrong!\n";
        this->error = true;
        return;
    }
    std::string subCommand = parser.getPositionalArgument(0);
    if (subCommand != "list-certs") {
        std::cout << "Unsupported subcommand given!\n";
        this->error = true;
        return;
    }
    std::cout << "Using database directory: " << initDir << "\n";

    // init NSS
    const char *certPrefix = ""; // certutil -P option  --- can leave this empty
    SECStatus rv =
        NSS_Initialize(initDir.c_str(), certPrefix, certPrefix, "secmod.db", 0);
    if (rv != SECSuccess) {
        this->error = true;
        std::cout << "NSS init failed!\n";
        return;
    }

    std::cout << "Listing certificates...\n";
    this->listCertificates();
}

DBTool::~DBTool()
{
    // shutdown nss
    if (NSS_Shutdown() != SECSuccess) {
        std::cout << "NSS Shutdown failed!\n";
    }
}

bool
DBTool::getError()
{
    return this->error;
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

} /* end namespace nss_tool */
