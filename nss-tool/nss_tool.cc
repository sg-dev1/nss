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
Usage()
{
    std::cerr << "Usage: nss <command> <subcommand> [options]" << std::endl;
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Error: At least a command must be given!" << std::endl;
        Usage();
        return 1;
    }

    std::string command(argv[1]);
    if ("db" != command) {
        Usage();
        return 1;
    }
    int result = 0;
    PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);

    std::vector<std::string> arguments(argv + 2, argv + argc);
    nss_tool::DBTool tool;
    if (!tool.Run(arguments)) {
        Usage();
        tool.Usage();
        result = 1;
    }

    PR_Cleanup();

    return result;
}
