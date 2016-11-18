// C includes
#include <stdlib.h>

// C++ includes
#include <iostream>
#include <string>

// own includes
#include "argparse.h"
#include "nss_tool.h"

static void usage() {
  std::cout << "Usage: nss <command> <subcommand> [options]\n";
  std::cout << "   nss help  to print this usage!\n";
  std::cout << "----------------------------------\n";
  std::cout << "   nss cert certreq [--ascii] [--subject <RFC 1485 subject string>] [--outfile <path>\n]";
}

int main(int argc, char **argv) {
  //argParseTest(argc, argv);
  if(argc < 2) {
    std::cout << "Error: At least a command must be given!\n";
    usage();
    exit(1);
  }

  std::string command(argv[1]);
  NSS_Tool tool = NSS_Tool(std::string("."));

  if("help" == command) {
    usage();
    exit(0);
  }
  else if("cert" == command) {
    stringMap optionsMap ( {{"--ascii", "none"}, {"--subject", "string"}, {"--outfile", "string"}} );
    std::vector<std::string> arguments(argv+2, argv + argc);

    ArgParse p = ArgParse(arguments, 1, optionsMap);
    std::vector<std::string> args = p.getArgs();
    stringMap ret = p.getOptionsMap();

    if("certreq" == args[0]) {
      bool ascii;
      std::string subject;

      if(p.existsKeyInResultMap("--ascii")) {
        ascii = true;
      }
      else {
        ascii = false;
      }

      if(p.existsKeyInResultMap("--outfile")) {
        tool.openOutFileForWriting(ret["--outfile"]);
      }

      if(p.existsKeyInResultMap("--subject")) {
        subject = ret["--subject"];
      }
      else {
        subject = std::string("CN=test user, O=test organization, C=AT");
      }

      // now generate or get private key
      tool.generatePrivateKey(std::string("default"));  // TODO nickname handling
      tool.listPrivateKeys();

      tool.certificateRequest(subject, ascii); // subject must be according RFC 1485
    }
    else {
      // Error
      std::cout << "Error: Unkown subcommand '" << args[0] << "' for cert command given!\n";
      usage();
    }
  }
  else if("db" == command) {

  }
  else {
    std::cout << "Error: Unknown command '" << command << "' given!\n";
    usage();
    exit(1);
  }

  /*
  NSS_Tool tool = NSS_Tool(std::string("."));
  tool.generatePrivateKey();
  tool.listPrivateKeys();

  tool.certificateRequest(std::string("CN=test user, O=test organization, C=AT"), true); // subject must be according RFC 1485

  std::cout << "printing certificates...\n";
  tool.listCertificates();
  */
}
