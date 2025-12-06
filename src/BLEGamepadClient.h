#pragma once

#include "BLEAutoScan.h"
#include "BLEControllerRegistry.h"

// export headers
#include "xbox/XboxController.h"
#include "steam/SteamController.h"

class BLEGamepadClient {
 public:
  BLEGamepadClient() = delete;

  static void init(bool initBLE = true);
  static void deleteBonds();
  static void enableDebugLog();
  static BLEAutoScan* getAutoScan();

  friend class BLEAbstractController;

 private:
  static bool _initialized;
  static TaskHandle_t _autoScanTask;
  static TaskHandle_t _scanCallbackTask;
  static QueueHandle_t _callbackQueue;
  static BLEAutoScan _autoScan;
  static BLEControllerRegistry _controllerRegistry;
};
