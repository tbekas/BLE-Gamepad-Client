#pragma once

#include<NimBLEDevice.h>

class ScanCallbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
  void onScanEnd(const NimBLEScanResults& results, int reason) override;
};
