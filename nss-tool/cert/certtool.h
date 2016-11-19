/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef NSS_TOOL_CERT_H
#define NSS_TOOL_CERT_H

#include "../nss_include.h"

namespace nss_tool {

class CertTool {
 public:
  CertTool();

  void usage();
  int getError();

 private:
};

} /* end namespace nss_tool */

#endif /* end of include guard: NSS_TOOL_CERT_H */
