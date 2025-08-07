#pragma once

#include <NimBLEDevice.h>
#include "BLEAdapterRegistry.h"
#include "BLEControllerRegistry.h"

class BLEScanCallbacksImpl : public NimBLEScanCallbacks {
 public:
  BLEScanCallbacksImpl(BLEAdapterRegistry& adapterRegistry, BLEControllerRegistry& controllerRegistry);

      void onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
  void onScanEnd(const NimBLEScanResults& results, int reason) override;
 private:
  BLEAdapterRegistry& _adapterRegistry;
  BLEControllerRegistry& _controllerRegistry;
};
