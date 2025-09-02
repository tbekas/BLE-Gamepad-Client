#pragma once

#include "BLEAutoScanner.h"
#include "BLEControllerRegistry.h"
#include "BLEBaseController.h"

// additional headers
#include "XboxController.h"
#include "XboxControlsEvent.h"
#include "XboxBatteryEvent.h"
#include "XboxVibrationsCommand.h"


class BLEGamepadClient {
 public:
  BLEGamepadClient() = delete;

  static void initBLEDevice();
  static void enableAutoScan();
  static void disableAutoScan();
  static bool isAutoScanEnabled();
  static void deleteBonds();

  friend class BLEBaseController;

 private:
  static TaskHandle_t _autoScanTask;
  static BLEAutoScanner _autoScanner;
  static BLEControllerRegistry _controllerRegistry;
};
