/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef keytool_h__
#define keytool_h__

#include <string>
#include <vector>
#include "argparse.h"

class KeyTool {
 public:
  bool Run(const std::vector<std::string>& arguments);

  void Usage();

 private:
  bool GeneratePrivPubKeyPair(const ArgParser& parser);
};

#endif  // keytool_h__
