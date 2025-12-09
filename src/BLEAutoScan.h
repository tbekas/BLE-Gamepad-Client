#pragma once

#include "BLEControllerRegistry.h"
#include "messages.h"

class BLEAutoScan {
 public:
  BLEAutoScan(BLEControllerRegistry& controllerRegistry,
              TaskHandle_t& autoScanTask,
              QueueHandle_t& userCallbackQueue);
  ~BLEAutoScan();

  void enable();
  void disable();
  bool isEnabled() const;
  bool isScanning() const;
  void notify() const;
  void onScanStarted(const std::function<void()>& callback);
  void onScanStopped(const std::function<void()>& callback);

  friend class BLEUserCallbackRunner;

 private:
  class ScanCallbacksImpl final : public NimBLEScanCallbacks {
   public:
    explicit ScanCallbacksImpl(BLEAutoScan& autoScan);
    void onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
    void onScanEnd(const NimBLEScanResults& results, int reason) override;
    BLEAutoScan& _autoScan;
  };

  void callOnScanStarted();
  void callOnScanStopped();
  void _sendUserCallbackMsg(const BLEUserCallback& msg) const;
  void _startScan(NimBLEScan* pScan, bool highDuty);
  void _stopScan(NimBLEScan* pScan);

  static void _autoScanTaskFn(void* pvParameters);

  bool _enabled = true;
  TaskHandle_t& _autoScanTask;
  unsigned long _startTimeMs;
  BLEControllerRegistry& _controllerRegistry;
  ScanCallbacksImpl _scanCallbacksImpl;
  std::function<void()> _onScanStarted;
  std::function<void()> _onScanStopped;
  QueueHandle_t& _userCallbackQueue;
};
