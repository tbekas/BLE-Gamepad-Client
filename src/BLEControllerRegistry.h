#pragma once

#include <atomic>
#include <memory>
#include <NimBLEDevice.h>
#include "BLEBaseController.h"

enum BLEClientEventKind : uint8_t {
  BLEClientBonded = 0, BLEClientDisconnected = 1, BLEClientConnectingFailed = 2, BLEClientBondingFailed = 3,
};

struct BLEClientEvent {
  NimBLEAddress address;
  BLEClientEventKind kind;

  explicit operator std::string() const;
};

class BLEControllerRegistry {
 public:
  BLEControllerRegistry(TaskHandle_t& autoScanTask);
  ~BLEControllerRegistry();

  void registerController(BLEBaseController* pCtrl);
  void deregisterController(BLEBaseController* pCtrl);
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

  BLEBaseController* _getController(NimBLEAddress address) const;
  bool _allocateController(const NimBLEAdvertisedDevice* pAdvertisedDevice);
  bool _deallocateController(BLEBaseController* pCtrl);
  void _sendClientEvent(const BLEClientEvent& msg) const;
  static void _callbackTaskFn(void* pvParameters);
  static void _clientEventConsumerFn(void* pvParameters);

  TaskHandle_t& _autoScanTask;
  TaskHandle_t _callbackTask;
  QueueHandle_t _clientEventQueue;
  TaskHandle_t _clientEventConsumerTask;
  SemaphoreHandle_t _connectionSlots;
  std::atomic<std::vector<BLEBaseController*>*> _controllers;
  ClientCallbacks _clientCallbacks;
};
