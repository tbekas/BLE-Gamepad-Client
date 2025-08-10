#pragma once

#include "BLEAutoScanner.h"
#include "BLEController.h"
#include "BLEControllerMatcher.h"
#include "BLEControllerRegistry.h"

class BLEGamepadClient {
 public:
  BLEGamepadClient() = delete;

  static bool init();
  static bool deinit();
  static bool isInitialized();
  static void enableAutoScan();
  static void disableAutoScan();
  static bool isAutoScanEnabled();
  static void deleteBonds();
  static bool addControllerModel(const BLEControllerModel& model);

  friend class BLEController;

 private:
  static bool _initialized;
  static TaskHandle_t _autoScanTask;
  static BLEAutoScanner _autoScanner;
  static BLEControllerMatcher _matcher;
  static BLEControllerRegistry _controllerRegistry;
};
