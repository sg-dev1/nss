/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <string>
#include <vector>

#include <prinit.h>

#include "common/argparse.h"
#include "db/dbtool.h"

static void
usage()
{
    std::cout << "Usage: nss <command> <subcommand> [options]\n";
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        std::cout << "Error: At least a command must be given!\n";
        usage();
        return 1;
    }

    std::string command(argv[1]);
    if ("db" != command) {
        usage();
        return 1;
    }
    PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);

    std::vector<std::string> arguments(argv + 2, argv + argc);
    nss_tool::DBTool tool;
    if (!tool.run(arguments)) {
        usage();
        tool.usage();
        return 1;
    }

    PR_Cleanup();
}
