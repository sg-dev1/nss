/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "nss_common.h"
#include "error.h"
#include "../nss_include.h"

namespace nss_tool {

int initNSSTool(std::string initDir) {
  PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);
  const char *certPrefix = "";  // certutil -P option  --- can leave this empty
  SECStatus rv = NSS_Initialize(initDir.c_str(), certPrefix, certPrefix, "secmod.db", 0);
  if(rv != SECSuccess) {
    return ERROR_NSS_INIT;
  }

  return NO_ERROR;
}

void setDBPin(ScopedPK11SlotInfo slot) {
  PK11_InitPin(slot.get(), (char *)NULL, "");
}

void setDBPin(ScopedPK11SlotInfo slot, std::string password) {
  PK11_InitPin(slot.get(), (char *)NULL, password.c_str());
}


int shutdownNSSTool(void) {
  if(NSS_Shutdown() != SECSuccess) {
    return ERROR_NSS_SHUTDOWN;
  }
  PR_Cleanup();
  return NO_ERROR;
}

} /* end namespace nss_tool */
