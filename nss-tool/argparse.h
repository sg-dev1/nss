#ifndef ARGPARSE_H
#define ARGPARSE_H

// C++ includes
#include <vector>
#include <string>
#include <exception>
#include <unordered_map>

typedef std::unordered_map<std::string,std::string> stringMap;

/**
 * Class for parsing commandline options
 * Parsing is done in constructor and throws ParsingException on error.
 * Results can be retrieved via getArgs() and getOptionsMap() functions.
 * All results are std::string, so f.e. conversion to int must be done afterwards,
 *  however parameters are checked if valid string.
 */
class ArgParse {
public:
  ArgParse(const std::vector<std::string> _arguments, int requiredArgs, const stringMap _optionsMap);

  std::vector<std::string> getArgs(void);
  stringMap getOptionsMap(void);
  bool existsKeyInResultMap(const std::string& key);

private:
  const std::vector<std::string> arguments;   // readonly copy of argv without program name, command etc.
  int requiredArgs;
  std::vector<std::string> args;
  const stringMap optionsMap;                 // read only input map
  stringMap resultMap;                        // output map with longopt -- paramValue mapping (or null if no param)
};

/*
 * Contains a small example implementation
 */
void argParseTest(int argc, char **argv);

#endif /* end of include guard: ARGPARSE_H */
