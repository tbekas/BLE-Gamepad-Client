#pragma once

#include <NimBLERemoteCharacteristic.h>
#include <string>

class Utils {
 public:
  static std::string remoteCharToStr(NimBLERemoteCharacteristic* pChar) {
    auto str = pChar->toString();

    std::string::size_type pos = str.find('\n');
    if (pos != std::string::npos) {
      return str.substr(0, pos);
    }
    return str;
  }
};
