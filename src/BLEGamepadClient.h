#pragma once

#include "BLEAutoScan.h"
#include "BLEControllerRegistry.h"
#include "BLEUserCallbackRunner.h"

// export headers
#include "xbox/XboxController.h"
#include "steam/SteamController.h"

class BLEGamepadClient {
 public:
  BLEGamepadClient() = delete;

  static void init(bool deleteBonds = true);
  static void enableDebugLog();
  static BLEAutoScan* getAutoScan();

  friend class BLEAbstractController;

 private:
  static void _initSelf();
  static bool _initialized;
  static TaskHandle_t _autoScanTask;
  static QueueHandle_t _userCallbackQueue;
  static BLEAutoScan _autoScan;
  static BLEControllerRegistry _controllerRegistry;
  static BLEUserCallbackRunner _userCallbackRunner;
};
