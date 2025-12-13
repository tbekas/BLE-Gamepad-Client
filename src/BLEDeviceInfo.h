#pragma once

#include <NimBLEDevice.h>
#include <string>
#include <vector>

struct BLEDeviceInfo {
  std::string manufacturerName;
  std::string modelName;
  std::string serialNumber;
  std::string firmwareRevision;
  std::vector<uint8_t> pnpId;

  static BLEDeviceInfo readDeviceInfo(NimBLEClient* pClient);

  explicit operator std::string() const {
    return "BLEDeviceInfo manufacturerName: " + manufacturerName + ", modelName: " + modelName +
           ", serialNumber: " + serialNumber + ", firmwareRevision: " + firmwareRevision;
  }
};
