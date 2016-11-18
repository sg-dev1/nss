#ifndef ARGPARSE_H
#define ARGPARSE_H

// C++ includes
#include <vector>
#include <string>
#include <exception>
#include <unordered_map>

/**
 * Class for parsing commandline options
 * Parsing is done in constructor and throws ParsingException on error.
 * Results can be retrieved via getArgs() and getOptionsMap() functions.
 * All results are std::string, so f.e. conversion to int must be done afterwards,
 *  however parameters are checked if valid string.
 */
class ArgParse {
public:
  ArgParse(const std::vector<std::string> _arguments, int requiredArgs, const std::vector<std::string> _optionsList);

  std::string get(std::string argument);
  bool getBool(std::string argument);
  std::string getString(std::string argument);
  int getInt(std::string argument);

private:
  typedef std::tuple<std::string, std::string> argTuple_t;

  const std::vector<std::string> arguments;   // readonly copy of argv without program name, command etc.
  int requiredArgs;
  const std::vector<std::string> optionsList; // readonly list of options
  std::vector<argTuple_t> parsedArgs;         // contains the parsed results
};

/*
 * Contains a small example implementation
 */
void argParseTest(int argc, char **argv);

#endif /* end of include guard: ARGPARSE_H */
