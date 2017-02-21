/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <string>
#include <vector>

#include <prinit.h>

#include "argparse.h"
#include "db/dbtool.h"
#include "key/keytool.h"

static void Usage() {
  std::cerr << "Usage: nss <command> <subcommand> [options]" << std::endl;
  std::cerr << "       nss db [--path <directory>] <commands>" << std::endl;
  std::cerr << "       nss key [--path <directory>] <commands>" << std::endl;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    Usage();
    return 1;
  }

  std::string command(argv[1]);
  if (command != "db" && command != "key") {
    Usage();
    return 1;
  }

  int exit_code = 0;
  PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);

  std::vector<std::string> arguments(argv + 2, argv + argc);
  if (command == "db") {
    DBTool tool;
    if (!tool.Run(arguments)) {
      tool.Usage();
      exit_code = 1;
    }
  } else { /* command == "key" */
    KeyTool tool;
    if (!tool.Run(arguments)) {
      tool.Usage();
      exit_code = 1;
    }
  }

  PR_Cleanup();

  return exit_code;
}
