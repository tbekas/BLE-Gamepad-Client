#pragma once

#include "BLEAutoScanner.h"
#include "BLEControllerRegistry.h"

// export headers
#include "xbox/XboxController.h"
#include "steam/SteamController.h"

class BLEGamepadClient {
 public:
  BLEGamepadClient() = delete;

  static void initBLEDevice();
  static void enableAutoScan();
  static void disableAutoScan();
  static bool isAutoScanEnabled();
  static void deleteBonds();

  friend class BLEAbstractController;

 private:
  static TaskHandle_t _autoScanTask;
  static BLEAutoScanner _autoScanner;
  static BLEControllerRegistry _controllerRegistry;
};
