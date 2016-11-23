/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <string>
#include <vector>

class ArgObject
{
  public:
    ArgObject(std::string _argument, std::string _description);
    bool isPresent();
    std::string getArgument();
    std::string getValue();

    //  this should only be visible in this file (for the ArgParser)
    void setValue(std::string newValue);
    void setPresent();

  private:
    const std::string argument;
    std::string value;
    const std::string description;

    bool _isPresent = false;
};

class ArgParser
{
  public:
    bool parse(const std::vector<std::string> arguments);
    void add(ArgObject obj);

    int getPositionalArgumentCount();
    std::string getPositionalArgument(int pos);

  private:
    std::vector<ArgObject> programArgs;
    std::vector<std::string> positionalArgs;
};

#endif /* end of include guard: ARGPARSE_H */
