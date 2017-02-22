/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "util.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <prerror.h>

#if defined(__unix__)
#include <termios.h>
#include <unistd.h>
#elif defined(WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

static std::string GetPassword(const std::string &promt) {
  std::cout << promt << std::endl;

#if defined(__unix__)
  termios oldt;
  tcgetattr(STDIN_FILENO, &oldt);
  termios newt = oldt;
  newt.c_lflag &= ~ECHO;
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#elif defined(WIN32) || defined(_WIN64)
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
  DWORD mode = 0;
  GetConsoleMode(hStdin, &mode);
  SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
#endif

  std::string pw;
  std::getline(std::cin, pw);

#if defined(__unix__)
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#elif defined(WIN32) || defined(_WIN64)
  SetConsoleMode(hStdin, mode);
#endif

  return pw;
}

static char *GetModulePassword(PK11SlotInfo *slot, int retry, void *arg) {
  if (arg == nullptr) {
    return nullptr;
  }

  PwData *pwData = reinterpret_cast<PwData *>(arg);

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
    pw = GetPassword("Enter new password: ");
    pwComp = GetPassword("Re-enter password: ");

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
  if (!PK11_NeedLogin(slot.get())) {
    return true;
  }

  PK11_SetPasswordFunc(&GetModulePassword);
  std::string pw = GetPassword("Enter your password: ");
  PwData pwData = {PW_PLAINTEXT, const_cast<char *>(pw.c_str())};
  SECStatus rv = PK11_Authenticate(slot.get(), true /*loadCerts*/, &pwData);
  if (rv != SECSuccess) {
    std::cerr << "Could not authenticate to token "
              << PK11_GetTokenName(slot.get()) << ". Failed with error "
              << PR_ErrorToName(PR_GetError()) << std::endl;
    return false;
  }
  std::cout << std::endl;

  return true;
}

std::string StringToHex(const ScopedSECItem &input) {
  std::stringstream ss;
  ss << "0x";
  for (size_t i = 0; i < input->len; i++) {
    ss << std::hex << std::setfill('0') << std::setw(2)
       << static_cast<int>(input->data[i]);
  }

  return ss.str();
}
