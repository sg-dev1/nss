/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NSS_TOOL_DB_H
#define NSS_TOOL_DB_H

#include <string>
#include <vector>

namespace nss_tool
{

class DBTool
{
  public:
    ~DBTool();

    /**
     * DBTool's main method
     * Returns true on success and false on error
    */
    bool run(std::vector<std::string> arguments);

    void usage();

  private:
    void listCertificates();
};

} /* end namespace nss_tool */

#endif /* end of include guard: NSS_TOOL_DB_H */
