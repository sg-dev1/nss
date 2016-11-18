// C++ includes
#include <iostream>

#include "argparse.h"
#include "util.h"

typedef std::unordered_map<std::string,std::string> stringMap;

class ParsingException: public std::exception {
  virtual const char* what() const throw()
  {
    return "A parsing exception occured!";
  }
} parsingException;

bool ArgParse::existsKeyInResultMap(const std::string& key) {
  stringMap::const_iterator mapIter = this->resultMap.find(key);
  if ( mapIter == this->resultMap.end() ) {
    return false;
  }
  else {
    return true;
  }
}

ArgParse::ArgParse(const std::vector<std::string> _arguments, int requiredArgs, const stringMap _optionsMap) :
 arguments(_arguments), optionsMap(_optionsMap) {
   this->requiredArgs = requiredArgs;
   int remainingCmdTokens = this->arguments.size() - requiredArgs;
   if(remainingCmdTokens < 0) {
     std::cout << "Error: Not enough required arguments given!\n";
     throw parsingException;
   }
   args = std::vector<std::string>(this->requiredArgs);
   this->resultMap = stringMap();

   // parse args
   for(int i=0; i < this->requiredArgs; i++) {
     this->args[i] = std::string(this->arguments.at(i));
   }

   // parse options and parameters
   for(uint64_t i=this->requiredArgs; i < this->arguments.size(); i++) {
     std::string opt = std::string(this->arguments.at(i));
     std::string param;

     stringMap::const_iterator got = this->optionsMap.find(opt);
     if ( got == this->optionsMap.end() ) {
       std::cout << "Error: option " << opt << " not found or no valid option!\n";
       throw parsingException;
     }
     else {
       std::string type = got->second;
       if(type != "none") {
         i++;
         if(i >= this->arguments.size()) {
           std::cout << "Error: The option " << opt << " requires a parameter of type " << type << "\n";
           std::cout << "But no parameter was given!\n";
           throw parsingException;
         }

         param = std::string(this->arguments.at(i));
         if(type == "int") {
           // check type
           if( ! isInteger(param) ) {
             std:: cout << "Error! Given parameter " << param << " was no integer!";
             throw parsingException;
           }
         }
         else if(type != "string") {  // here could also be added other types: e.g. type != "string" || type != "bool"
             std::cout << "Error: invalid value " << opt << " in option Map found!\n";
             throw parsingException;
         }
       }

       // save type
       if (!existsKeyInResultMap(opt)) {
         this->resultMap[opt] = param;
       }
       else {
         // the parsed option was already given
         std::cout << "Error: The option " << opt << " was given at least twice!\n";
         throw parsingException;
       }
     }
   }
 }

std::vector<std::string> ArgParse::getArgs(void) {
  return this->args;
}

stringMap ArgParse::getOptionsMap(void) {
  return this->resultMap;
}

void argParseTest(int argc, char **argv) {
  stringMap optionsMap ( {{"--foo", "none"}, {"--bar", "int"}, {"--blubb", "string"}} );

  std::vector<std::string> arguments(argv+1, argv + argc);
  //for(std::string str: arguments) {
  //  std::cout << str << "\n";
  //}

  ArgParse p = ArgParse(arguments, 2, optionsMap);
  std::vector<std::string> args = p.getArgs();

  for(uint64_t i=0; i < args.size(); i++) {
    std::cout << "Cmd arg at position " << i << ": " << args[i] << "\n";
  }

  std::cout << "Printing cmdOption,value mapping:\n";
  stringMap ret = p.getOptionsMap();
  for(std::pair<std::string, std::string> pair : ret) {
    std::cout << pair.first << " " << pair.second << " \n";
  }
}
