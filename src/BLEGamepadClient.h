#pragma once

#include "BLEAutoScanner.h"
#include "BLEControllerAdapterRegistry.h"
#include "BLEControllerRegistry.h"
#include "BLEController.h"

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
  static bool addControllerAdapter(const BLEControllerAdapter& adapter);

 friend class BLEController;

 private:
  static bool _initialized;
  static TaskHandle_t _autoScanTask;
  static BLEAutoScanner _autoScanner;
  static BLEControllerAdapterRegistry _adapterRegistry;
  static BLEControllerRegistry _controllerRegistry;
};
