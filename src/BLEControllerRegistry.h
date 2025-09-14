#pragma once

#include <atomic>
#include <memory>
#include <NimBLEDevice.h>
#include "BLEBaseController.h"

enum BLEClientStatusMsgKind : uint8_t { BLEClientConnected = 0, BLEClientDisconnected = 1 };

struct BLEClientStatus {
  NimBLEAddress address;
  BLEClientStatusMsgKind kind;

  explicit operator std::string() const;
};

class BLEControllerRegistry {
 public:
  BLEControllerRegistry(TaskHandle_t& autoScanTask);
  ~BLEControllerRegistry();

  void registerController(BLEBaseController* pCtrl);
  void deregisterController(const BLEBaseController* controller);
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
  bool _reserveController(const NimBLEAdvertisedDevice* pAdvertisedDevice);
  bool _releaseController(NimBLEAddress address);
  static void _clientStatusConsumerFn(void* pvParameters);

  TaskHandle_t& _autoScanTask;
  QueueHandle_t _clientStatusQueue;
  TaskHandle_t _clientStatusConsumerTask;
  SemaphoreHandle_t _connectionSlots;
  std::atomic<std::shared_ptr<std::vector<BLEBaseController*>>> _controllers;
  // std::atomic<std::vector<BLEBaseController*>> _controllers;
  ClientCallbacks _clientCallbacks;
};
