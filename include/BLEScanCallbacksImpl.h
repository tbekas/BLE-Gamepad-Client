#pragma once

#include<NimBLEDevice.h>

class BLEScanCallbacksImpl : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
  void onScanEnd(const NimBLEScanResults& results, int reason) override;
};
