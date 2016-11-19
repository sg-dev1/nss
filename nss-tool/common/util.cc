/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "util.h"

PRBool ItemIsPrintableASCII(const SECItem *item)
{
    unsigned char *src = item->data;
    unsigned int len = item->len;
    while (len-- > 0) {
        unsigned char uc = *src++;
        if (uc < 0x20 || uc > 0x7e)
            return PR_FALSE;
    }
    return PR_TRUE;
}

/* Caller ensures that dst is at least item->len*2+1 bytes long */
void SECItemToHex(const SECItem *item, char *dst)
{
    if (dst && item && item->data) {
        unsigned char *src = item->data;
        unsigned int len = item->len;
        for (; len > 0; --len, dst += 2) {
            sprintf(dst, "%02x", *src++);
        }
        *dst = '\0';
    }
}

/*
 * http://stackoverflow.com/a/2844907
 * http://stackoverflow.com/a/2845275
 */
bool isInteger(const std::string& s) {
  bool isNumber = true;
  for(std::string::const_iterator k = s.begin(); k != s.end(); ++k)
    isNumber &= isdigit(*k);

  return isNumber;
}
