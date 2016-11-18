// C includes
#include <stdlib.h>
#include <assert.h>
#if XP_UNIX
#include <termios.h>
#endif

// C++ includes
#include <iostream>

#include "nss_include.h"
#include "nss_tool.h"
#include "util.h"

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

/*
 * GenerateRandom
 * taken from https://developer.mozilla.org/en-US/docs/Mozilla/Projects/NSS/NSS_Sample_Code/NSS_Sample_Code_Utililies_1
 * and modified a bit
 */
SECStatus GenerateRandom(void)
{
    const int rsize = 60;
    unsigned char rbuf[rsize];

    char meter[] = {
                   "\r|                                |" };
    int            fd,  count;
    int            c;
    SECStatus      rv                  = SECSuccess;

#ifdef XP_UNIX
    cc_t           orig_cc_min;
    cc_t           orig_cc_time;
    tcflag_t       orig_lflag;
    struct termios tio;
#endif

    fprintf(stderr, "To generate random numbers, "
            "continue typing until the progress meter is full:\n\n");
    fprintf(stderr, "%s", meter);
    fprintf(stderr, "\r|");

    /* turn off echo on stdin & return on 1 char instead of NL */
    fd = fileno(stdin);

#ifdef XP_UNIX
    tcgetattr(fd, &tio);
    orig_lflag = tio.c_lflag;
    orig_cc_min = tio.c_cc[VMIN];
    orig_cc_time = tio.c_cc[VTIME];
    tio.c_lflag &= ~ECHO;
    tio.c_lflag &= ~ICANON;
    tio.c_cc[VMIN] = 1;
    tio.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &tio);
#endif

    /* Get random noise from keyboard strokes */
    count = 0;
    while (count < rsize) {
#ifdef XP_UNIX
        c = getc(stdin);
#else
        c = getch();
#endif
        if (c == EOF) {
            rv = SECFailure;
            break;
        }
        *(rbuf + count) = c;
        if (count == 0 || c != *(rbuf + count -1)) {
            count++;
            fprintf(stderr, "*");
        }
    }
    rbuf[count] = '\0';

    PK11_RandomUpdate(rbuf, sizeof rbuf);
    memset(rbuf, 0, sizeof rbuf);

    fprintf(stderr, "\n\nFinished.  Press enter to continue: ");
    while ((c = getc(stdin)) != '\n' && c != EOF)
        ;
    if (c == EOF)
        rv = SECFailure;
    fprintf(stderr, "\n");

#ifdef XP_UNIX
    /* set back termio the way it was */
    tio.c_lflag = orig_lflag;
    tio.c_cc[VMIN] = orig_cc_min;
    tio.c_cc[VTIME] = orig_cc_time;
    tcsetattr(fd, TCSAFLUSH, &tio);
#endif
    return rv;
}

NSS_Tool::NSS_Tool(std::string initDir) {
  PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);
  const char *certPrefix = "";  // certutil -P option  --- can leave this empty
  SECStatus rv = NSS_Initialize(initDir.c_str(), certPrefix, certPrefix, "secmod.db", 0);
  if(rv != SECSuccess) {
    throw NSS_Exception("Error in NSS_Initialize!");
  }

  this->slot = PK11_GetInternalKeySlot();
  if(slot == NULL) {
    throw NSS_Exception("Error in PK11_GetInternalKeySlot");
  }

  this->certHandle = CERT_GetDefaultCertDB();

  // if creating new db and init it with empty password
  // TODO only call it if we are creating a new db
  // TODO add support for init db with password
	PK11_InitPin(slot, (char *)NULL, "");
}

NSS_Tool::~NSS_Tool() {
  if(this->privKey != NULL) {
    SECKEY_DestroyPrivateKey(this->privKey);
  }

  if(this->pubKey != NULL) {
    SECKEY_DestroyPublicKey(this->pubKey);
  }

  if(this->subject != NULL) {
    CERT_DestroyName(this->subject);
  }

  SECITEM_FreeItem(&this->certReqDER, PR_FALSE);

  if(this->slot != NULL) {
    PK11_FreeSlot(this->slot);
  }

  if(NSS_Shutdown() != SECSuccess) {
    exit(1);
  }

  PR_Cleanup();
}

void NSS_Tool::generatePrivateKey(std::string nickName) {
  CK_MECHANISM_TYPE mechanism;
	PK11RSAGenParams rsaparams;
	//SECKEYPQGParams *dsaparams = NULL;
	void *params;

  // generate random seed
  /* TODO commented out for testing ...
  SECStatus rv = GenerateRandom();
  if(rv != SECSuccess) {
    throw NSS_Exception("Error in GenerateRandom!");  // TODO maybe use other exception types for internal failures
  }
  */

  // for now use RSA
  rsaparams.keySizeInBits = 1024;
  rsaparams.pe = 0x010001;
  mechanism = CKM_RSA_PKCS_KEY_PAIR_GEN;
  params = &rsaparams;

  assert(this->slot != NULL);
  this->privKey = PK11_GenerateKeyPairWithOpFlags(this->slot, mechanism, params, &this->pubKey,
      (PK11_ATTR_TOKEN | PK11_ATTR_SENSITIVE | PK11_ATTR_PRIVATE), 0, 0,
          NULL /*wincx*/);

  if(this->privKey == NULL) {
    throw NSS_Exception("Error in PK11_GenerateKeyPairWithOpFlags!");
  }

  // Set the nickname on the private key so that it
  // can be found later.
  SECStatus s = PK11_SetPrivateKeyNickname(this->privKey, nickName.c_str());
  if(s != SECSuccess) {
    throw NSS_Exception("Setting nickname for private key failed!");
  }
}

void NSS_Tool::loadPrivPubKeypair(std::string keysource) {
  CERTCertificate *keycert;
  keycert = CERT_FindCertByNicknameOrEmailAddr(this->certHandle, keysource.c_str());
  if(keycert == NULL) {
    keycert = PK11_FindCertFromNickname(keysource.c_str(), NULL);
    if(keycert == NULL) {
      throw NSS_Exception("Given keysource is neither a key-type nor a nickname");
    }
  }
  this->privKey = PK11_FindKeyByDERCert(this->slot, keycert, NULL);
  if(this->privKey != NULL) {
    this->pubKey = CERT_ExtractPublicKey(keycert);
  }
  if(this->pubKey == NULL) {
    CERT_DestroyCertificate(keycert);
    throw NSS_Exception("Could not get keys from given keysource");
  }

  this->keytype = this->privKey->keyType;
  if(this->subject == NULL) {
    this->subject = CERT_AsciiToName(keycert->subjectName);
  }

  CERT_DestroyCertificate(keycert);
}

void NSS_Tool::certificateRequest(std::string subjectString, bool ascii) {
  // SECOidTag hashAlgTag
  // const char *phone
  // int ascii,
  // const char *emailAddrs
  // const char *dnsNames,
  // certutilExtnList extnList
  // const char *extGeneric,
  // PRBool pssCertificate
  // /*out*/ SECItem *result

  if(this->subject == NULL) {
    this->subject = CERT_AsciiToName(subjectString.c_str());
    if(this->subject == NULL) {
      throw NSS_Exception("Improperly formatted subject name!");
    }
  }

  assert(this->certHandle);
  assert(this->slot);
  assert(this->pubKey);
  assert(this->privKey);
  assert(this->subject);

  CERTSubjectPublicKeyInfo *spki;
  CERTCertificateRequest *cr;
  SECItem *encoding;
  SECOidTag signAlgTag;
  SECStatus rv;
  PLArenaPool *arena;
  void *extHandle;
  SECItem signedReq = { siBuffer, NULL, 0 };

  arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);
  if (!arena) {
    throw NSS_Exception("out of memory");
  }

  /* Create info about public key */
  spki = SECKEY_CreateSubjectPublicKeyInfo(this->pubKey);
  if(spki == NULL) {
    PORT_FreeArena(arena, PR_FALSE);
    throw NSS_Exception("Unable to create subject public key");
  }

  /* Change cert type to RSA-PSS, if desired. */
  // TODO see CertReq in certutil.c

  /* Generate certificate request */
  cr = CERT_CreateCertificateRequest(subject, spki, NULL);
  SECKEY_DestroySubjectPublicKeyInfo(spki);
  if(cr == NULL) {
    PORT_FreeArena(arena, PR_FALSE);
    throw NSS_Exception("Unable to make certificate request");
  }

  extHandle = CERT_StartCertificateRequestAttributes(cr);
  if (extHandle == NULL) {
    PORT_FreeArena(arena, PR_FALSE);
    throw NSS_Exception("CERT_StartCertificateRequestAttributes failed!");
  }
  // TODO here Extensions can be added --> see AddExtensions in certutl's certext.c
  CERT_FinishExtensions(extHandle);
  CERT_FinishCertificateRequestAttributes(cr);

  /* Der encode the request */
  encoding = SEC_ASN1EncodeItem(arena, NULL, cr,
                                  SEC_ASN1_GET(CERT_CertificateRequestTemplate));
  CERT_DestroyCertificateRequest(cr);
  if(encoding == NULL) {
    PORT_FreeArena(arena, PR_FALSE);
    throw NSS_Exception("DER encoding of request failed!");
  }

  /* Sign the request */
  SECOidTag hashAlgTag = SEC_OID_SHA512;   // TODO make it configurable -- see secutil's SECU_StringToSignatureAlgTag function in basicutil.c
  signAlgTag = SEC_GetSignatureAlgorithmOidTag(this->keytype, hashAlgTag);
  if(!signAlgTag) {
    PORT_FreeArena(arena, PR_FALSE);
    throw NSS_Exception("Unknown Key or Hash type!");
  }

  rv = SEC_DerSignData(arena, &signedReq, encoding->data, encoding->len,
                         this->privKey, signAlgTag);
  if(rv != SECSuccess) {
    PORT_FreeArena(arena, PR_FALSE);
    throw NSS_Exception("Signing of data failed!");
  }

  /* Encode request in specified format */
  if(ascii) {
    char *obuffer;
    char *header, *name, *email, *org, *state, *country;
    const char *phone = "(not specified)";  // TODO this must be given as input parameter to the function

    // first try with arena
    obuffer = NSSBase64_EncodeItem(NULL, NULL, 0, &signedReq);
    if(obuffer == NULL) {
      goto oom;
    }

    name = CERT_GetCommonName(this->subject);
    if (!name) {
        name = PORT_Strdup("(not specified)");
    }

  //  if (!phone)
  //      phone = "(not specified)";

    email = CERT_GetCertEmailAddress(this->subject);
    if (!email)
        email = PORT_Strdup("(not specified)");

    org = CERT_GetOrgName(this->subject);
    if (!org)
        org = PORT_Strdup("(not specified)");

    state = CERT_GetStateName(this->subject);
    if (!state)
        state = PORT_Strdup("(not specified)");

    country = CERT_GetCountryName(this->subject);
    if (!country)
        country = PORT_Strdup("(not specified)");

    header = PR_smprintf(
        "\nCertificate request generated by nss command\n"
        "Phone: %s\n\n"
        "Common Name: %s\n"
        "Email: %s\n"
        "Organization: %s\n"
        "State: %s\n"
        "Country: %s\n\n"
        "%s\n",
        phone, name, email, org, state, country, NS_CERTREQ_HEADER);

    PORT_Free(name);
    PORT_Free(email);
    PORT_Free(org);
    PORT_Free(state);
    PORT_Free(country);

    if(header) {
      char *trailer = PR_smprintf("\n%s\n", NS_CERTREQ_TRAILER);
      if (trailer) {
          PRUint32 headerLen = PL_strlen(header);
          PRUint32 obufLen = PL_strlen(obuffer);
          PRUint32 trailerLen = PL_strlen(trailer);
          SECITEM_AllocItem(NULL, &this->certReqDER,
                            headerLen + obufLen + trailerLen);
          if (this->certReqDER.data) {
              PORT_Memcpy(this->certReqDER.data, header, headerLen);
              PORT_Memcpy(this->certReqDER.data + headerLen, obuffer, obufLen);
              PORT_Memcpy(this->certReqDER.data + headerLen + obufLen,
                          trailer, trailerLen);
          }
          PR_smprintf_free(trailer);
      }
      PR_smprintf_free(header);
    }
    PORT_Free(obuffer);
  }
  else {
    (void)SECITEM_CopyItem(NULL, &this->certReqDER, &signedReq);
  }

  if(this->certReqDER.data == NULL) {
  oom:
    PORT_FreeArena(arena, PR_FALSE);
    throw NSS_Exception("out of memory");
  }

  PORT_FreeArena(arena, PR_FALSE);

  // now print the request to a file or write it to stdout
  PRInt32 written = PR_Write(this->outFile, this->certReqDER.data, this->certReqDER.len);
  if (written < 0 || (PRUint32)written != this->certReqDER.len) {
    throw NSS_Exception("Writing certificate request data failed!");
  }
  std::cout << "\n";
}

void NSS_Tool::listPrivateKeys() {
  const char* nickName = "";       // this is not necessarily needed
  SECKEYPrivateKeyList *list;
  SECKEYPrivateKeyListNode *node;
  int count = 0;

  list = PK11_ListPrivateKeysInSlot(this->slot);
  if(list == NULL) {
    throw NSS_Exception("Error in PK11_ListPrivateKeysInSlot");
  }

  // list keys
  for (node = PRIVKEY_LIST_HEAD(list);
       !PRIVKEY_LIST_END(node, list);
       node = PRIVKEY_LIST_NEXT(node)) {
    char *keyName;
    static const char orphan[] = { "(orphan)" };

    //if (keyType != nullKey && keyType != node->key->keyType)
    //    continue;
    keyName = PK11_GetPrivateKeyNickname(node->key);
    if (!keyName || !keyName[0]) {
        /* Try extra hard to find nicknames for keys that lack them. */
        CERTCertificate *cert;
        PORT_Free((void *)keyName);
        keyName = NULL;
        cert = PK11_GetCertFromPrivateKey(node->key);
        if (cert) {
            if (cert->nickname && cert->nickname[0]) {
                keyName = PORT_Strdup(cert->nickname);
            } else if (cert->emailAddr && cert->emailAddr[0]) {
                keyName = PORT_Strdup(cert->emailAddr);
            }
            CERT_DestroyCertificate(cert);
        }
    }

    if (!keyName) {
        keyName = (char *)orphan;
        std::cout << "Warning: Current Key has no nickname!\n";
    }

    //std::cout << "key with name " << keyName << ": ";

    // ### BEGIN PrintKey(PR_STDOUT, keyName, count, node->key, NULL);
    // #define MAX_CKA_ID_STR_LEN 40
    const int MAX_CKA_ID_STR_LEN = 40;
    char ckaIDbuf[MAX_CKA_ID_STR_LEN + 4];
    SECKEYPrivateKey *key = node->key;
    SECItem *ckaID = PK11_GetLowLevelKeyIDForPrivateKey(key);
    if(!ckaID) {
      std::cout << "Error: PK11_GetLowLevelKeyIDForPrivateKey failed!\n";
      continue;
    }
    else if(ItemIsPrintableASCII(ckaID)) {
      int len = PR_MIN(MAX_CKA_ID_STR_LEN, ckaID->len);
      ckaIDbuf[0] = '"';
      memcpy(ckaIDbuf + 1, ckaID->data, len);
      ckaIDbuf[1 + len] = '"';
      ckaIDbuf[2 + len] = '\0';
    }
    else {
      // print it in hex
      //std::cout << "Printing key info it in hex...\n";
      const int MAX_CKA_ID_BIN_LEN = 20;
      SECItem idItem = *ckaID;
      if (idItem.len > MAX_CKA_ID_BIN_LEN)
          idItem.len = MAX_CKA_ID_BIN_LEN;
      SECItemToHex(&idItem, ckaIDbuf);
    }

    PR_fprintf(PR_STDOUT, "<%2d, name:%-8.8s> %-8.8s %-42.42s %s\n", count, keyName,
               keyTypeName[key->keyType], ckaIDbuf, nickName);
    SECITEM_ZfreeItem(ckaID, PR_TRUE);

    // ### END PrintKey
    if (keyName != (char *)orphan)
        PORT_Free((void *)keyName);
    count++;
  }
  SECKEY_DestroyPrivateKeyList(list);
}

void NSS_Tool::listCertificates() {
  CERTCertList *list;
  CERTCertListNode *node;

  list = PK11_ListCerts(PK11CertListAll, NULL);
  for (node = CERT_LIST_HEAD(list); !CERT_LIST_END(node, list);
    node = CERT_LIST_NEXT(node)) {
    //SECU_PrintCertNickname(node, stdout);
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

void NSS_Tool::createCertificate() {
  // see certutil.c's CreateCert()
}


void NSS_Tool::addCertificate() {
  // see certutil.c's AddCert()
}

void NSS_Tool::outputCertificate() {
  // see certutil.c  line 3523 --> use PR_Write
}

void NSS_Tool::openOutFileForWriting(std::string filename) {
  outFile = PR_Open(filename.c_str(),
      PR_CREATE_FILE | PR_RDWR | PR_TRUNCATE, 00660);
  if (!outFile) {
    PR_fprintf(PR_STDERR,
        "nss:  unable to open \"%s\" for writing (%ld, %ld).\n",
        filename.c_str(), PR_GetError(), PR_GetOSError());
    throw NSS_Exception("Opening outfile failed!");
  }
}
