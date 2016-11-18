#ifndef NSS_TOOL_H
#define NSS_TOOL_H



#endif /* end of include guard: NSS_TOOL_H */


// C++ includes
#include <string>
#include <exception>

#include "nss_include.h"

#define NS_CERTREQ_HEADER "-----BEGIN NEW CERTIFICATE REQUEST-----"
#define NS_CERTREQ_TRAILER "-----END NEW CERTIFICATE REQUEST-----"

class NSS_Exception : public std::exception {
public:
  explicit NSS_Exception(const std::string& message) : _message("NSS_Exception: " + message) {}
  virtual const char* what() const throw() {
      return this->_message.c_str();
  }
private:
  std::string _message;
};

class NSS_Tool {
public:
  NSS_Tool(std::string initDir);
  ~NSS_Tool();

  void generatePrivateKey(std::string nickName);
  void loadPrivPubKeypair(std::string keysource);

  // subjectString must be according RFC 1485
  void certificateRequest(std::string subjectString, bool ascii); // after creating it the certificate can f.e. be written to a file
  void createCertificate();
  void addCertificate();
  void outputCertificate();

  void listPrivateKeys();
  void listCertificates();

  void openOutFileForWriting(std::string filename);

private:
  PK11SlotInfo *slot = NULL;
  CERTCertDBHandle *certHandle = NULL;
  SECKEYPrivateKey *privKey = NULL;
  SECKEYPublicKey *pubKey = NULL;
  CERTName *subject = NULL;
  KeyType keytype = rsaKey;
  SECOidTag hashAlgTag = SEC_OID_UNKNOWN;
  SECItem certReqDER = { siBuffer, NULL, 0 };
  PRFileDesc *outFile = PR_STDOUT;
};
