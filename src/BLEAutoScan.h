#pragma once

#include "BLEControllerRegistry.h"

class BLEAutoScan {
 public:
  enum class BLEAutoScanNotification : uint8_t {
   Auto = 0, ScanEnd = 1
  };

  BLEAutoScan(TaskHandle_t& startStopScanTask,
              TaskHandle_t& scanCallbackTask,
              BLEControllerRegistry& controllerRegistry);
  ~BLEAutoScan() = default;

  void enable();
  void disable();
  bool isEnabled() const;
  bool isScanning() const;
  void notify() const;
  void onScanStart(const std::function<void()>& callback);
  void onScanStop(const std::function<void()>& callback);

 private:
  class ScanCallbacks final : public NimBLEScanCallbacks {
   public:
    explicit ScanCallbacks(BLEAutoScan& autoScan);
    void onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
    void onScanEnd(const NimBLEScanResults& results, int reason) override;
    BLEAutoScan& _autoScan;
  };

  void _startHighDuty(NimBLEScan* pScan);
  void _startLowDuty(NimBLEScan* pScan);
  void _stopScan(NimBLEScan* pScan);

  static void _startStopScanTaskFn(void* pvParameters);
  static void _scanCallbackTaskFn(void* pvParameters);

  bool _enabled = true;
  TaskHandle_t& _startStopScanTask;
  TaskHandle_t& _scanCallbackTask;
  unsigned long _startTimeMs;
  unsigned int _restartCount;
  BLEControllerRegistry& _controllerRegistry;
  ScanCallbacks _bleScanCallbacks;
  std::function<void()> _onScanStart;
  std::function<void()> _onScanStop;
};
