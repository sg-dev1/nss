/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "nspr.h"
#include "nss.h"
#include "prenv.h"

#include <cstdlib>

#define GTEST_HAS_RTTI 0
#include "gtest/gtest.h"

std::string g_working_dir_path;

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  g_working_dir_path = ".";
  char* workdir = PR_GetEnvSecure("NSS_GTEST_WORKDIR");
  if (workdir) g_working_dir_path = workdir;

  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-d")) {
      g_working_dir_path = argv[i + 1];
      ++i;
    }
  }

  PR_Init(PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);
  int rv = RUN_ALL_TESTS();

  PR_Cleanup();

  return rv;
}
