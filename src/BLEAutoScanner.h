#pragma once

#include "BLEControllerRegistry.h"

class BLEAutoScanner {
 public:
  BLEAutoScanner(TaskHandle_t& autoScanTask,
                 BLEControllerRegistry& controllerRegistry);
  ~BLEAutoScanner() = default;

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
  BLEControllerRegistry& _controllerRegistry;
  ScanCallbacks _scanCallbacks;
};
