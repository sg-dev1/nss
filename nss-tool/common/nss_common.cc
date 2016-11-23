/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "nss_common.h"
#include "../nss_include.h"

namespace nss_tool
{

void
setDBPin(ScopedPK11SlotInfo slot)
{
    PK11_InitPin(slot.get(), (char *)NULL, "");
}

void
setDBPin(ScopedPK11SlotInfo slot, std::string password)
{
    PK11_InitPin(slot.get(), (char *)NULL, password.c_str());
}

} /* end namespace nss_tool */
