/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "util.h"

#include <iostream>
#include <string>

#include <prerror.h>

#if defined(__unix__)
#include <termios.h>
#include <unistd.h>
// http://stackoverflow.com/a/6899073
#define GET_PASSWORD(promt, pw)              \
  do {                                       \
    std::cout << promt << std::endl;         \
                                             \
    termios oldt;                            \
    tcgetattr(STDIN_FILENO, &oldt);          \
    termios newt = oldt;                     \
    newt.c_lflag &= ~ECHO;                   \
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); \
                                             \
    std::getline(std::cin, pw);              \
                                             \
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); \
  } while (0)

#elif defined(WIN32) || defined(_WIN64)
#include <Windows.h>
// http://stackoverflow.com/a/6899073
#define GET_PASSWORD(promt, pw)                         \
  do {                                                  \
    std::cout << promt << std::endl;                    \
                                                        \
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);     \
    DWORD mode = 0;                                     \
    GetConsoleMode(hStdin, &mode);                      \
    SetConsoleMode(hStdin, mode &(~ENABLE_ECHO_INPUT)); \
                                                        \
    std::getline(std::cin, pw);                         \
                                                        \
    SetConsoleMode(hStdin, mode);                       \
  } while (0)

#else
/* fallback implementation */
#define GET_PASSWORD(promt, pw)      \
  do {                               \
    std::cout << promt << std::endl; \
    std::getline(std::cin, pw);      \
  } while (0)
#endif

static char *GetModulePassword(PK11SlotInfo *slot, int retry, void *arg) {
  PwData *pwData = reinterpret_cast<PwData *>(arg);

  if (pwData == nullptr) {
    return nullptr;
  }

  if (retry > 0) {
    std::cerr << "Incorrect password/PIN entered." << std::endl;
    return nullptr;
  }

  switch (pwData->source) {
    case PW_NONE:
    case PW_FROMFILE:
      std::cerr << "Password input method not supported." << std::endl;
      return nullptr;
    case PW_PLAINTEXT:
      return PL_strdup(pwData->data);
    default:
      break;
  }

  std::cerr << "Password check failed:  No password found." << std::endl;
  return nullptr;
}

bool InitSlotPassword(void) {
  ScopedPK11SlotInfo slot(PK11_GetInternalKeySlot());
  if (slot.get() == nullptr) {
    std::cerr << "Error: Init PK11SlotInfo failed!" << std::endl;
    return false;
  }

  std::cout << "Enter a password which will be used to encrypt your keys."
            << std::endl
            << std::endl;
  std::string pw, pwComp;

  while (true) {
    GET_PASSWORD("Enter new password: ", pw);
    GET_PASSWORD("Re-enter password: ", pwComp);

    if (pw == pwComp) {
      break;
    }

    std::cerr << "Passwords do not match. Try again." << std::endl;
  }

  SECStatus rv = PK11_InitPin(slot.get(), nullptr, pw.c_str());
  if (rv != SECSuccess) {
    std::cerr << "Init db password failed." << std::endl;
    return false;
  }

  return true;
}

bool DBLoginIfNeeded(const ScopedPK11SlotInfo &slot) {
  PK11_SetPasswordFunc(&GetModulePassword);
  if (PK11_NeedLogin(slot.get())) {
    std::string pw;
    GET_PASSWORD("Enter your password: ", pw);
    PwData pwData = {PW_PLAINTEXT, const_cast<char *>(pw.c_str())};
    SECStatus rv = PK11_Authenticate(slot.get(), true /*loadCerts*/, &pwData);
    if (rv != SECSuccess) {
      std::cerr << "Could not authenticate to token "
                << PK11_GetTokenName(slot.get()) << ". Failed with error "
                << PR_ErrorToName(PR_GetError()) << std::endl;
      return false;
    }
    std::cout << std::endl;
  }

  return true;
}
