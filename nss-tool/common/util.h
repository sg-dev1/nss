/**
 * This module contains utility functions
 * partly taken from certutil
 */
#ifndef UTIL_H
#define UTIL_H

// C++ includes
#include <string>

#include "../nss_include.h"

static const char *const keyTypeName[] = {
    "null", "rsa", "dsa", "fortezza", "dh", "kea", "ec"
};
PRBool ItemIsPrintableASCII(const SECItem *item);
/* Caller ensures that dst is at least item->len*2+1 bytes long */
void SECItemToHex(const SECItem *item, char *dst);

bool isInteger(const std::string& s);

#endif /* end of include guard: UTIL_H */
