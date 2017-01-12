/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <string>
#include <vector>
#include <unordered_map>

class ArgParser
{
  public:
    ArgParser(const std::vector<std::string>& arguments);

    bool
    Has(std::string arg)
    {
        return programArgs_.count(arg) > 0;
    }
    std::string
    Get(std::string arg)
    {
        return programArgs_[arg];
    }

    int
    GetPositionalArgumentCount()
    {
        return positionalArgs_.size();
    }
    std::string
    GetPositionalArgument(int pos)
    {
        return positionalArgs_.at(pos);
    }

  private:
    std::unordered_map<std::string, std::string> programArgs_;
    std::vector<std::string> positionalArgs_;
};

#endif /* end of include guard: ARGPARSE_H */
