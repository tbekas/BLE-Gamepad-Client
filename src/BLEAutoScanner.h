#pragma once

#include "BLEControllerMatcher.h"
#include "BLEControllerRegistry.h"

class BLEAutoScanner {
 public:
  BLEAutoScanner(TaskHandle_t& autoScanTask,
                 BLEControllerRegistry& controllerRegistry,
                 BLEControllerMatcher& matcher);
  ~BLEAutoScanner() = default;

  bool init();
  bool deinit();
  bool isInitialized() const;
  void enableAutoScan();
  void disableAutoScan();
  bool isAutoScanEnabled() const;

 private:
  class ScanCallbacks final : public NimBLEScanCallbacks {
   public:
    explicit ScanCallbacks(BLEAutoScanner& autoScanner);
    void onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
    void onScanEnd(const NimBLEScanResults& results, int reason) override;
    BLEAutoScanner& _autoScanner;
  };

  static void _autoScanTaskFn(void* pvParameters);

  bool _initialized = false;
  bool _autoScanEnabled = true;
  TaskHandle_t& _autoScanTask;
  BLEControllerMatcher& _matcher;
  BLEControllerRegistry& _controllerRegistry;
  ScanCallbacks _scanCallbacks;
};
