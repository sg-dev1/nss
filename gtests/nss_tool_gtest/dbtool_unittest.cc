/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <string>
#include <vector>

#include "dbtool.h"
#include "gtest/gtest.h"

namespace nss_test {

class DBToolTest : public ::testing::Test {};

TEST_F(DBToolTest, DBCreateTest) {
  const std::vector<std::string> arguments = {"--create"};

  DBTool tool;
  EXPECT_TRUE(tool.Run(arguments));
}

}  // namespace nss_test
