#ifndef NSS_INCLUDE_H
#define NSS_INCLUDE_H

/* NSPR Headers */
#include <prprf.h>        // contains PR_*print* functions
#include <prtypes.h>      // nss types (PRBool, PRInt32)
#include <prio.h>         // file/socket etc. operations (e.g. PR_Read, PR_Recv)
#include <prthread.h>
#include <prinit.h>
#include <prerror.h>
//#include <prlong.h>     // 64-bit integer support (for compilers who don't support)
//#include <prtime.h>     // NSPR date and time functions

/* NSS headers */
#include <secoid.h>       // SECOID_* functions (ASN.1 OID functions)
#include <secmodt.h>      // PK11* and SECMOD_* types/macros
#include <sechash.h>      // hash functions, all starting with HASH_*
#include <nss.h>          // contains NSS_Init and NSS_Shutdown functions, some macros
#include <pk11pub.h>      // various PK11_* and some SECMOD_* (SECMOD_OpenNewSlot etc.) functions
#include <secasn1.h>      // encoding/decoding of ASN.1 using BER/DER; SEC_* functions
#include <cert.h>         // public data structures and functions for the certificate library; CERT_* functions
#include <cryptohi.h>     // DSAU_* (DER encode/decode/sign (EC)DSA signatures), SEC_* (sign data),
                          // SGN_* (signing context), VFY_* (verification context) functions
#include <certdb.h>       // cert db handling functions: SEC_* and CERT_AddTempCertToPerm
#include <keyhi.h>
#include <nssb64.h>

#endif /* end of include guard: NSS_INCLUDE_H */
