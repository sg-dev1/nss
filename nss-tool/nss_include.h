/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef NSS_INCLUDE_H
#define NSS_INCLUDE_H

/* NSPR Headers */
#include <prerror.h>
#include <prinit.h>
#include <prio.h>   // file/socket etc. operations (e.g. PR_Read, PR_Recv)
#include <prprf.h>  // contains PR_*print* functions
#include <prthread.h>
#include <prtypes.h>  // nss types (PRBool, PRInt32)
//#include <prlong.h>     // 64-bit integer support (for compilers who don't
//support)
//#include <prtime.h>     // NSPR date and time functions

/* NSS headers */
#include <cert.h>  // public data structures and functions for the certificate library; CERT_* functions
#include <cryptohi.h>  // DSAU_* (DER encode/decode/sign (EC)DSA signatures), SEC_* (sign data),
#include <nss.h>  // contains NSS_Init and NSS_Shutdown functions, some macros
#include <pk11pub.h>  // various PK11_* and some SECMOD_* (SECMOD_OpenNewSlot etc.) functions
#include <secasn1.h>  // encoding/decoding of ASN.1 using BER/DER; SEC_* functions
#include <sechash.h>  // hash functions, all starting with HASH_*
#include <secmodt.h>  // PK11* and SECMOD_* types/macros
#include <secoid.h>   // SECOID_* functions (ASN.1 OID functions)
// SGN_* (signing context), VFY_* (verification context) functions
#include <certdb.h>  // cert db handling functions: SEC_* and CERT_AddTempCertToPerm
#include <keyhi.h>
#include <nssb64.h>

#endif /* end of include guard: NSS_INCLUDE_H */