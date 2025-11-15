#pragma once

#include <atomic>
#include <memory>
#include <NimBLEDevice.h>
#include "BLEBaseController.h"

enum BLEClientEventKind : uint8_t {
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

class BLEControllerRegistry {
 public:
  BLEControllerRegistry(TaskHandle_t& autoScanTask, TaskHandle_t& scanCallbackTask);
  ~BLEControllerRegistry();

  void registerController(BLEAbstractController* pCtrl);
  void deregisterController(BLEAbstractController* pCtrl, bool startStopScan = false);
  void tryConnectController(const NimBLEAdvertisedDevice* pAdvertisedDevice);
  unsigned int getAvailableConnectionSlotCount() const;

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

  BLEAbstractController* _getController(NimBLEAddress address) const;
  bool _allocateController(const NimBLEAdvertisedDevice* pAdvertisedDevice);
  bool _deallocateController(BLEAbstractController* pCtrl);
  void _sendClientEvent(const BLEClientEvent& msg) const;
  void _runCtrlCallback(const BLEAbstractController* pCtrl) const;
  void _startStopScan() const;
  void _runScanCallback() const;
  static void _callbackTaskFn(void* pvParameters);
  static void _clientEventConsumerFn(void* pvParameters);

  TaskHandle_t& _startStopScanTask;
  TaskHandle_t& _scanCallbackTask;
  TaskHandle_t _callbackTask;
  QueueHandle_t _clientEventQueue;
  TaskHandle_t _clientEventConsumerTask;
  SemaphoreHandle_t _connectionSlots;
  std::atomic<std::vector<BLEAbstractController*>*> _controllers;
  ClientCallbacks _clientCallbacks;
};
