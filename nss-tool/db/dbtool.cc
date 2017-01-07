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
PrintFlags(unsigned int flags)
{
    std::stringstream ss;
    if ((flags & CERTDB_VALID_CA) && !(flags & CERTDB_TRUSTED_CA) && !(flags & CERTDB_TRUSTED_CLIENT_CA)) {
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

void
DBTool::Usage()
{
    std::cout << "    nss db [--db <directory>] list-certs" << std::endl;
}

bool
DBTool::Run(std::vector<std::string> arguments)
{
    ArgParser parser;
    if (!parser.Parse(arguments)) {
        std::cerr << "Parsing error!" << std::endl;
        return false;
    }

    std::string initDir(".");
    if (parser.Has("--db")) {
        initDir = parser.Get("--db");
    }
    if (parser.GetPositionalArgumentCount() != 1) {
        std::cerr << "The dbtool requires at least one subcommand given!" << std::endl;
        return false;
    }
    std::string subCommand = parser.GetPositionalArgument(0);
    if (subCommand != "list-certs") {
        std::cerr << "Unsupported subcommand " << subCommand << " given!" << std::endl;
        return false;
    }
    std::cout << "Using database directory: " << initDir << std::endl;

    // init NSS
    const char *certPrefix = ""; // certutil -P option  --- can leave this empty
    SECStatus rv =
        NSS_Initialize(initDir.c_str(), certPrefix, certPrefix, "secmod.db", 0);
    if (rv != SECSuccess) {
        std::cerr << "NSS init failed!" << std::endl;
        return false;
    }

    std::cout << "Listing certificates: " << std::endl;
    this->ListCertificates();

    // shutdown nss
    if (NSS_Shutdown() != SECSuccess) {
        std::cerr << "NSS Shutdown failed!" << std::endl;
    }

    return true;
}

void
DBTool::ListCertificates()
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

        if (node->appData && static_cast<char *>(node->appData)[0]) {
            name = static_cast<char *>(node->appData);
        } else if (cert->nickname && cert->nickname[0]) {
            name = cert->nickname;
        } else if (cert->emailAddr && cert->emailAddr[0]) {
            name = cert->emailAddr;
        }

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
        std::cout << std::setw(60) << std::left << name << " " << trusts << std::endl;
    }
}

} /* end namespace nss_tool */
