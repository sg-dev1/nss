#ifndef NSS_TOOL_DB_H
#define NSS_TOOL_DB_H

#include <string>
#include <vector>
#include "../nss_include.h"
#include "../common/error.h"

//namespace nss_tool {

class DBTool {
public:
  DBTool(std::vector<std::string> arguments);

  void usage();
  int getError();

private:
  void listCertificates();

  bool parserError = false;
  bool cmdNotFound = false;
};

//} /* end namespace nss_tool */

#endif /* end of include guard: NSS_TOOL_DB_H */
