#pragma once

#include <NimBLEDevice.h>
#include <atomic>
#include <memory>
#include "BLEAbstractController.h"
#include "messages.h"

class BLEControllerRegistry {
 public:
  struct AllocationInfo {
    unsigned int allocated = 0;
    unsigned int notAllocated = 0;
  };

  BLEControllerRegistry(TaskHandle_t& autoScanTask, QueueHandle_t& userCallbackQueue);
  ~BLEControllerRegistry();

  void registerController(BLEAbstractController* pCtrl);
  void deregisterController(BLEAbstractController* pCtrl, bool notifyAutoScan = false);
  void tryConnectController(const NimBLEAdvertisedDevice* pAdvertisedDevice);
  AllocationInfo getAllocationInfo() const;

 private:
  class ClientCallbacksImpl final : public NimBLEClientCallbacks {
   public:
    explicit ClientCallbacksImpl(BLEControllerRegistry& controllerRegistry);
    void onConnect(NimBLEClient* pClient) override;
    void onConnectFail(NimBLEClient* pClient, int reason) override;
    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override;
    void onDisconnect(NimBLEClient* pClient, int reason) override;
    BLEControllerRegistry& _controllerRegistry;
  };

  enum class ClientEventKind : uint8_t {
    ClientConnectionFailed = 3,
    ClientBondingFailed = 4,
    ClientConnected = 0,
    ClientBonded = 1,
    ClientDisconnected = 2,
  };

  struct ClientEvent {
    NimBLEAddress address;
    ClientEventKind kind;

    explicit operator std::string() const;
  };

  BLEAbstractController* _findController(const NimBLEAddress& address) const;
  BLEAbstractController* _findAndAllocateController(const NimBLEAdvertisedDevice* pAdvertisedDevice);
  void _sendClientEvent(const ClientEvent& msg) const;
  void _sendUserCallbackMsg(const BLEAbstractController* pCtrl) const;
  void _startScan() const;
  void _notifyAutoScan(BLEAutoScanNotification notification = BLEAutoScanNotification::Auto) const;
  void _sendUserCallbackMsg(const BLEUserCallback& msg) const;
  static void _clientEventConsumerFn(void* pvParameters);

  std::vector<BLEAbstractController*> _controllers;
  SemaphoreHandle_t _controllersMutex;
  TaskHandle_t& _autoScanTask;
  QueueHandle_t& _userCallbackQueue;
  QueueHandle_t _clientEventQueue;
  TaskHandle_t _clientEventConsumerTask;
  ClientCallbacksImpl _clientCallbacksImpl;
};
