/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "argparse.h"
#include <assert.h>
#include <algorithm>
#include <iostream>
#include "util.h"

#define REQUIRED_ARG "__REQUIRED_ARG_"
#define NOT_FOUND "_NOT_FOUND_"

// Note: required args need not appear directly at the beginning, can also
// appear somewhere at the end
ArgParse::ArgParse(const std::vector<std::string> _arguments, int requiredArgs,
                   const std::vector<std::string> _optionsList)
    : arguments(_arguments), optionsList(_optionsList) {
  const std::string prefix("--");
  this->requiredArgs = requiredArgs;
  int remainingCmdTokens = this->arguments.size() - requiredArgs;
  int requiredArgsFound = 0;
  if (remainingCmdTokens < 0) {
    std::cout << "Error: Not enough required arguments given!\n";
    this->parsingError = true;
    return;
  }

  // parse options and parameters
  for (uint64_t i = 0; i < this->arguments.size(); i++) {
    std::string arg = std::string(this->arguments.at(i));
    std::cout << "parsing: " << arg << "...\n";
    if (arg.compare(0, prefix.size(), prefix) == 0) {  // is option ?
      if (std::find(this->optionsList.begin(), this->optionsList.end(), arg) ==
          this->optionsList.end()) {  // option not found ?
        std::cout << "Error: Option " << arg << " is not a valid option!\n";
        this->parsingError = true;
        return;
      }

      if (i + 1 < this->arguments.size() &&
          this->arguments.at(i + 1).compare(0, prefix.size(),
                                            prefix)) {  // is option argument ?
        this->parsedArgs.push_back(
            std::make_tuple(arg, std::string(this->arguments.at(i + 1))));
        i++;
      } else {
        this->parsedArgs.push_back(std::make_tuple(arg, std::string("true")));
      }
    } else {
      std::cout << "Required arg found: " << arg << "\n";
      this->parsedArgs.push_back(
          std::make_tuple(this->arguments.at(i), REQUIRED_ARG));
      requiredArgsFound++;
    }
  }

  // check number of required args
  if (this->requiredArgs != requiredArgsFound) {
    std::cout << "Error: " << this->requiredArgs << " requested, but "
              << requiredArgsFound << " found!\n";
    this->parsingError = true;
    return;
  }
}

std::string ArgParse::get(std::string argument) {
  for (uint64_t i = 0; i < this->parsedArgs.size(); i++) {
    argTuple_t current = this->parsedArgs.at(i);
    if (std::get<0>(current) == argument) {
      return std::get<1>(current);
    }
  }

  return std::string(NOT_FOUND);
}

bool ArgParse::getBool(std::string argument) {
  return this->get(argument) == "true";
}

std::string ArgParse::getString(std::string argument) {
  std::string result = this->get(argument);
  if (result == NOT_FOUND) {
    this->elementNotFound = true;
  }
  return result;
}

int ArgParse::getInt(std::string argument) {
  std::string value = this->get(argument);
  if (value == NOT_FOUND) {
    this->elementNotFound = true;
  }
  return atoi(value.c_str());
}

std::string ArgParse::getRequired(int pos) {
  if (pos >= this->requiredArgs) {
    this->outOfRangeError = true;
    return std::string("out of range");
  }

  int curPos = 0;
  for (uint64_t i = 0; i < this->parsedArgs.size(); i++) {
    argTuple_t current = this->parsedArgs.at(i);
#ifdef DEBUG
    std::cout << "current: (" << std::get<0>(current) << ", "
              << std::get<1>(current) << ")\n";
#endif /* DEBUG */
    if (std::get<1>(current) == REQUIRED_ARG) {
      if (curPos == pos) {  // found!
        return std::get<0>(current);
      }
      curPos++;
    }
  }

  assert(0);
  return std::string(NOT_FOUND);
}

int ArgParse::getError() {
  if (parsingError) {
    return ERROR_PARSING;
  } else if (elementNotFound) {
    return ERROR_ELEMENT_NOT_FOUND;
  } else {
    return NO_ERROR;
  }
}
