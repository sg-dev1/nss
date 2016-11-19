// C++ includes
#include <iostream>
#include <algorithm>

#include "argparse.h"
#include "util.h"

#define REQUIRED_ARG "__REQUIRED_ARG_"
#define NOT_FOUND "_NOT_FOUND_"

class ParsingException: public std::exception {
  virtual const char* what() const throw()
  {
    return "A parsing exception occured!";
  }
} parsingException;

class ElementNotFoundException: public std::exception {
  virtual const char* what() const throw()
  {
    return "The given element could not be found!";
  }
} elementNotFoundException;

// Note: required args need not appear directly at the beginning, can also appear somewhere at the end
ArgParse::ArgParse(const std::vector<std::string> _arguments, int requiredArgs, const std::vector<std::string> _optionsList) :
 arguments(_arguments), optionsList(_optionsList) {
   const std::string prefix("--");
   this->requiredArgs = requiredArgs;
   int remainingCmdTokens = this->arguments.size() - requiredArgs;
   int requiredArgsFound = 0;
   if(remainingCmdTokens < 0) {
     std::cout << "Error: Not enough required arguments given!\n";
     throw parsingException;
   }

   // parse options and parameters
   for(uint64_t i=0; i < this->arguments.size(); i++) {
     std::string arg = std::string(this->arguments.at(i));
     std::cout << "parsing: " << arg << "...\n";
     if( arg.compare(0, prefix.size(), prefix) == 0  ) {              // is option ?
       if(std::find(this->optionsList.begin(),
          this->optionsList.end(), arg) == this->optionsList.end()) { // option not found ?
              std::cout << "Error: Option " << arg << " is not a valid option!\n";
              throw parsingException;
       }

       if(i+1 < this->arguments.size() &&
        this->arguments.at(i+1).compare(0, prefix.size(), prefix)) {  // is option argument ?
         this->parsedArgs.push_back(std::make_tuple(arg, std::string(this->arguments.at(i+1))));
         i++;
       }
       else {
         this->parsedArgs.push_back(std::make_tuple(arg, std::string("true")));
       }
     }
     else {
       std::cout << "Required arg found: " << arg << "\n";
       this->parsedArgs.push_back(std::make_tuple(this->arguments.at(i), REQUIRED_ARG));
       requiredArgsFound++;
     }
   }

   // check number of required args
   if(this->requiredArgs != requiredArgsFound) {
     std::cout << "Error: " << this->requiredArgs <<
        " requested, but " << requiredArgsFound << " found!\n";
     throw parsingException;
   }
}

std::string ArgParse::get(std::string argument) {
   for(uint64_t i = 0; i < this->parsedArgs.size(); i++) {
     argTuple_t current = this->parsedArgs.at(i);
     if(std::get<0>(current) == argument) {
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
  if(result == NOT_FOUND) {
    throw elementNotFoundException;
  }
  return result;
}

int ArgParse::getInt(std::string argument) {
  std::string value = this->get(argument);
  if(value == NOT_FOUND) {
    throw elementNotFoundException;
  }
  return atoi(value.c_str());
}

void argParseTest(int argc, char **argv) {
  std::vector<std::string> arguments(argv+1, argv + argc);
  std::vector<std::string> options = {"--foo", "--bar", "--blubb"};

  ArgParse p = ArgParse(arguments, 2, options);
  std::string foo = p.get("--foo");
  std::string bar = p.get("--bar");
  std::string blubb = p.get("--blubb");
  // now it would be the application which must interpret the parse tokens
  // e.g. if they are boolean, int or string etc.
  std::cout << "--foo=" << foo << ", --bar=" << bar <<
    ", blubb=" << blubb << "\n\n";

  // here with the interpreted API
  bool fooBool = p.getBool("--foo");
  std::string barString = p.getString("--bar");
  int blubbInt = p.getInt("--blubb");
  std::cout << "--foo=" << fooBool << ", --bar=" << barString <<
    ", blubb=" << blubbInt << "\n\n";

}
