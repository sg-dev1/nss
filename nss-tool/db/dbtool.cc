/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dbtool.h"
#include <iostream>
#include "../common/argparse.h"
#include "../common/scoped_ptrs.h"
#include <memory>

namespace nss_tool
{

// taken from secutil.c
static void
printflags(char *trusts, unsigned int flags)
{
    if (flags & CERTDB_VALID_CA)
        if (!(flags & CERTDB_TRUSTED_CA) && !(flags & CERTDB_TRUSTED_CLIENT_CA))
            PORT_Strcat(trusts, "c");
    if (flags & CERTDB_TERMINAL_RECORD)
        if (!(flags & CERTDB_TRUSTED))
            PORT_Strcat(trusts, "p");
    if (flags & CERTDB_TRUSTED_CA)
        PORT_Strcat(trusts, "C");
    if (flags & CERTDB_TRUSTED_CLIENT_CA)
        PORT_Strcat(trusts, "T");
    if (flags & CERTDB_TRUSTED)
        PORT_Strcat(trusts, "P");
    if (flags & CERTDB_USER)
        PORT_Strcat(trusts, "u");
    if (flags & CERTDB_SEND_WARN)
        PORT_Strcat(trusts, "w");
    if (flags & CERTDB_INVISIBLE_CA)
        PORT_Strcat(trusts, "I");
    if (flags & CERTDB_GOVT_APPROVED_CA)
        PORT_Strcat(trusts, "G");
    return;
}

void
DBTool::usage()
{
    std::cout << "    nss [--dbdir] list-certs\n";
}

DBTool::DBTool(std::vector<std::string> arguments)
{
    // TODO (new) argparse
    ArgParser parser;
    std::shared_ptr<ArgObject> dbDir = std::make_shared<ArgObject>("--dbDir", "Sets the path of the database directory");
    //ArgObject dbDir("--dbDir", "Sets the path of the database directory");
    parser.add(dbDir);

    if (!parser.parse(arguments)) {
        // parsing error
        std::cout << "Parsing error!\n";
        this->error = true;
        return;
    }

    std::cout << "dbDir value=" << dbDir->getValue() + "\n";
    std::string initDir(".");
    if (dbDir->isPresent()) {
        std::cout << "setting initDir\n";
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
    CERTCertList *list;
    CERTCertListNode *node;

    list = PK11_ListCerts(PK11CertListAll, NULL);
    for (node = CERT_LIST_HEAD(list); !CERT_LIST_END(node, list);
         node = CERT_LIST_NEXT(node)) {
        CERTCertTrust trust;
        CERTCertificate *cert;
        char trusts[30];
        char *name = NULL;

        cert = node->cert;

        PORT_Memset(trusts, 0, sizeof(trusts));

        name = (char *)node->appData;
        if (!name || !name[0]) {
            name = cert->nickname;
        }
        if (!name || !name[0]) {
            name = cert->emailAddr;
        }

        if (CERT_GetCertTrust(cert, &trust) == SECSuccess) {
            printflags(trusts, trust.sslFlags);
            PORT_Strcat(trusts, ",");
            printflags(trusts, trust.emailFlags);
            PORT_Strcat(trusts, ",");
            printflags(trusts, trust.objectSigningFlags);
        } else {
            PORT_Memcpy(trusts, ",,", 3);
        }
        printf("%-60s %-5s\n", name == NULL ? "(NULL)" : name, trusts);
    }
    CERT_DestroyCertList(list);
}

} /* end namespace nss_tool */
