/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <string>
#include <vector>
#include <iostream>
#include "common/argparse.h"
#include "common/error.h"
#include "common/nss_common.h"
#include "db/dbtool.h"

static void usage() {
  std::cout << "Usage: nss <command> <subcommand> [options]\n";
}

int main(int argc, char **argv) {
  if(argc < 2) {
    std::cout << "Error: At least a command must be given!\n";
    usage();
    exit(1);
  }

  std::string command(argv[1]);
  std::vector<std::string> arguments(argv+2, argv + argc);

  if("help" == command) {
    usage();
    exit(1);
  }
  else if("db" == command) {
    nss_tool::DBTool tool(arguments);
    if(tool.getError() != NO_ERROR) {
      usage();
      tool.usage();
      exit(1);
    }
  }
  // some other commands
  else {
    std::cout << "Error: Unknown command '" << command << "' given!\n";
    usage();
    exit(1);
  }

  nss_tool::shutdownNSSTool();
}
