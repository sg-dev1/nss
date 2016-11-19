/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <iostream>
#include "dbtool.h"
#include "../common/argparse.h"
#include "../common/nss_common.h"
#include "../common/scoped_ptrs.h"

namespace nss_tool {

// taken from secutil.c
static void
printflags(char *trusts, unsigned int flags)
{
    if (flags & CERTDB_VALID_CA)
        if (!(flags & CERTDB_TRUSTED_CA) &&
            !(flags & CERTDB_TRUSTED_CLIENT_CA))
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

void DBTool::usage() {
  std::cout << "    nss [--dbdir] list-certs\n";
}

DBTool::DBTool(std::vector<std::string> arguments) {
  std::vector<std::string> options = {"--dbdir"};

  ArgParse parser = ArgParse(arguments, 1, options);
  if(parser.getError() != NO_ERROR) {
    this->parserError = true;
    return;
  }

  std::string dbDir = parser.getString("--dbdir");
  if(parser.getError() != NO_ERROR) {
    dbDir = std::string(".");
  }
  nss_tool::initNSSTool(dbDir);

  std::string subcommand = parser.getRequired(0);
  if("list-certs" == subcommand) {
    std::cout << "listing certificates...\n";
    this->listCertificates();
  }
  else if("list-keys" == subcommand) {
    std::cout << "listing keys...\n";
    // TODO
  }
  else if("import" == subcommand) {
    std::cout << "importing...\n";
    // TODO import cert etc.
  }
  else {
    this->cmdNotFound = true;
  }
}

int DBTool::getError() {
  if(this->parserError) {
    return ERROR_PARSING;
  }
  else if(this->cmdNotFound) {
    return ERROR_COMMAND_NOT_FOUND;
  }
  else {
    return NO_ERROR;
  }
}

void DBTool::listCertificates() {
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

    name = (char*) node->appData;
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
