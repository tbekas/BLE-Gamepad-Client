#pragma once

#include <NimBLEDevice.h>
#include <atomic>
#include <memory>
#include "BLEBaseController.h"

class BLEControllerRegistry {
 public:
  struct BLEControllerAllocationInfo {
    unsigned int allocated = 0;
    unsigned int notAllocated = 0;
  };

  BLEControllerRegistry(TaskHandle_t& autoScanTask, TaskHandle_t& scanCallbackTask);
  ~BLEControllerRegistry();

  void registerController(BLEAbstractController* pCtrl);
  void deregisterController(BLEAbstractController* pCtrl, bool notifyAutoScan = false);
  void tryConnectController(const NimBLEAdvertisedDevice* pAdvertisedDevice);
  BLEControllerAllocationInfo getControllerAllocationInfo() const;

 private:
  class ClientCallbacks final : public NimBLEClientCallbacks {
   public:
    explicit ClientCallbacks(BLEControllerRegistry& controllerRegistry);
    void onConnect(NimBLEClient* pClient) override;
    void onConnectFail(NimBLEClient* pClient, int reason) override;
    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override;
    void onDisconnect(NimBLEClient* pClient, int reason) override;
    BLEControllerRegistry& _controllerRegistry;
  };

  enum class BLEClientEventKind : uint8_t {
    BLEClientConnected = 0,
    BLEClientBonded = 1,
    BLEClientDisconnected = 2,
    BLEClientConnectingFailed = 3,
    BLEClientBondingFailed = 4,
  };

  struct BLEClientEvent {
    NimBLEAddress address;
    BLEClientEventKind kind;

    explicit operator std::string() const;
  };

  BLEAbstractController* _findController(NimBLEAddress address) const;
  BLEAbstractController* _findAndAllocateController(const NimBLEAdvertisedDevice* pAdvertisedDevice);
  void _sendClientEvent(const BLEClientEvent& msg) const;
  void _runCtrlCallback(const BLEAbstractController* pCtrl) const;
  void _startScan() const;
  void _notifyAutoScan() const;
  void _runScanCallback() const;
  static void _callbackTaskFn(void* pvParameters);
  static void _clientEventConsumerFn(void* pvParameters);

  TaskHandle_t& _startStopScanTask;
  TaskHandle_t& _scanCallbackTask;
  TaskHandle_t _callbackTask;
  QueueHandle_t _clientEventQueue;
  TaskHandle_t _clientEventConsumerTask;
  std::atomic<std::vector<BLEAbstractController*>*> _controllers;
  ClientCallbacks _clientCallbacks;
};
