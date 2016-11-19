/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef NSS_TOOL_NSS_COMMON_H
#define NSS_TOOL_NSS_COMMON_H

#include <string>
#include "scoped_ptrs.h"

namespace nss_tool {

int initNSSTool(std::string initDir);
int shutdownNSSTool(void);

void setDBPin(ScopedPK11SlotInfo slot);
void setDBPin(ScopedPK11SlotInfo slot, std::string password);

} /* end namespace nss_tool */

#endif /* end of include guard: NSS_TOOL_NSS_COMMON_H */
