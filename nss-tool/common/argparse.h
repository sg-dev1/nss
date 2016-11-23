/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <string>
#include <tuple>
#include <vector>

class ArgObject {
public:
  ArgObject(std::string argument, std::string description);
  bool isPresent();
  std::string getValue();

  //  this should only be visible in this file (for the ArgParser)
  void setValue(std::string newValue);
private:
  std::string argument;
  std::string value;
  std::string description;

  bool _isPresent;
};

class ArgParser {
public:

private:
};

/**
 * Class for parsing commandline options
 * Parsing is done in constructor and throws ParsingException on error.
 * Results can be retrieved via getArgs() and getOptionsMap() functions.
 * All results are std::string, so f.e. conversion to int must be done
 * afterwards,
 *  however parameters are checked if valid string.
 */
class ArgParse {
 public:
  ArgParse(const std::vector<std::string> _arguments, int requiredArgs,
           const std::vector<std::string> _optionsList);

  bool getBool(std::string argument);
  std::string getString(std::string argument);
  int getInt(std::string argument);

  std::string getRequired(int pos);

  int getError();

 private:
  typedef std::tuple<std::string, std::string> argTuple_t;

  std::string get(std::string argument);

  const std::vector<std::string>
      arguments;  // readonly copy of argv without program name, command etc.
  int requiredArgs;
  const std::vector<std::string> optionsList;  // readonly list of options
  std::vector<argTuple_t> parsedArgs;          // contains the parsed results

  bool parsingError = false;
  bool elementNotFound = false;
  bool outOfRangeError = false;
};

#endif /* end of include guard: ARGPARSE_H */
