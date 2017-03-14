/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef digest_tool_h__
#define digest_tool_h__

#include <string>
#include <vector>

class DigestTool {
 public:
  bool Run(const std::vector<std::string>& arguments);

 private:
  void Usage();
  bool Digest();
};

#endif  // digest_tool_h__
