#pragma once

#include "BLEAutoScanner.h"
#include "BLEControllerRegistry.h"

// export headers
#include "xbox/XboxController.h"
#include "steam/SteamController.h"

class BLEGamepadClient {
 public:
  BLEGamepadClient() = delete;

  static void init(bool initBLE = true);
  static void enableAutoScan();
  static void disableAutoScan();
  static bool isAutoScanEnabled();
  static void deleteBonds();
  static void enableDebugLog();

  friend class BLEAbstractController;

 private:
  static bool _initialized;
  static TaskHandle_t _autoScanTask;
  static BLEAutoScanner _autoScanner;
  static BLEControllerRegistry _controllerRegistry;
};
