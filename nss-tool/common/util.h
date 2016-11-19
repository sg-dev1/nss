/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/**
 * This module contains utility functions
 * partly taken from certutil
 */
#ifndef UTIL_H
#define UTIL_H

#include <string>
#include "../nss_include.h"

static const char *const keyTypeName[] = {"null", "rsa", "dsa", "fortezza",
                                          "dh",   "kea", "ec"};
PRBool ItemIsPrintableASCII(const SECItem *item);
/* Caller ensures that dst is at least item->len*2+1 bytes long */
void SECItemToHex(const SECItem *item, char *dst);

bool isInteger(const std::string &s);

#endif /* end of include guard: UTIL_H */
