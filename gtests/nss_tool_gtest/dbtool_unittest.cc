/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "dbtool.h"
#include "gtest/gtest.h"

extern std::string g_working_dir_path;

namespace nss_test {

class DBToolTest : public ::testing::Test {};

static void createDBFiles(void) {
  std::cout << "Creating db files in working dir: " << g_working_dir_path
            << std::endl;

  std::istringstream passwordInput("a\na\n");
  auto cinBuffer = std::cin.rdbuf();
  std::cin.rdbuf(passwordInput.rdbuf());

  const std::vector<std::string> arguments = {"--create"};
  DBTool tool;
  EXPECT_TRUE(tool.Run(arguments));

  std::cin.rdbuf(cinBuffer);
}

static void removeDBFiles(void) {
#if defined(WIN32) || defined(_WIN64)
  const std::string pathSep("\\");
#else
  const std::string pathSep("/");
#endif

  std::cout << "Removing db files..." << std::endl;

  std::string keyDBFile = g_working_dir_path + pathSep + "key3.db";
  std::remove(keyDBFile.c_str());
  std::string certDBFile = g_working_dir_path + pathSep + "cert8.db";
  std::remove(certDBFile.c_str());
  std::string secModDBFile = g_working_dir_path + pathSep + "secmod.db";
  std::remove(secModDBFile.c_str());
}

TEST_F(DBToolTest, DBCreateTest) {
  createDBFiles();
  removeDBFiles();
}

TEST_F(DBToolTest, ListCerts) {
  createDBFiles();

  const std::vector<std::string> arguments = {"--list-certs"};

  DBTool tool;
  EXPECT_TRUE(tool.Run(arguments));

  removeDBFiles();
}

TEST_F(DBToolTest, ListKeys) {
  createDBFiles();

  std::istringstream passwordInput("a\n");
  auto cinBuffer = std::cin.rdbuf();
  std::cin.rdbuf(passwordInput.rdbuf());

  const std::vector<std::string> arguments = {"--list-keys"};
  DBTool tool;
  EXPECT_TRUE(tool.Run(arguments));

  std::cin.rdbuf(cinBuffer);

  removeDBFiles();
}

TEST_F(DBToolTest, ChangePassword) {
  createDBFiles();

  std::istringstream passwordInput("a\nnewPw\nnewPw\n");
  auto cinBuffer = std::cin.rdbuf();
  std::cin.rdbuf(passwordInput.rdbuf());

  const std::vector<std::string> arguments = {"--change-password"};
  DBTool tool;
  EXPECT_TRUE(tool.Run(arguments));

  std::cin.rdbuf(cinBuffer);

  removeDBFiles();
}

}  // namespace nss_test
