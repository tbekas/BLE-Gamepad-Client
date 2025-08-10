#pragma once

#include <NimBLEDevice.h>
#include <list>

#include "BLEControllerInternal.h"
#include "BLEControllerMatcher.h"

enum BLEClientStatusMsgKind : uint8_t { BLEClientConnected = 0, BLEClientDisconnected = 1 };

struct BLEClientStatus {
  NimBLEAddress address;
  BLEClientStatusMsgKind kind;

  explicit operator std::string() const;
};

class BLEControllerRegistry {
 public:
  BLEControllerRegistry(TaskHandle_t& autoScanTask, BLEControllerMatcher& matcher);

  bool init();
  bool deinit();
  bool isInitialized();
  BLEControllerInternal* createController(NimBLEAddress allowedAddress);
  void connectController(NimBLEAddress address);
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

  BLEControllerInternal* _getController(NimBLEAddress address);
  bool _reserveController(NimBLEAddress address);
  bool _releaseController(NimBLEAddress address);
  static void _clientStatusConsumerFn(void* pvParameters);

  bool _initialized;
  TaskHandle_t& _autoScanTask;
  BLEControllerMatcher& _matcher;
  QueueHandle_t _clientStatusQueue;
  TaskHandle_t _clientStatusConsumerTask;
  SemaphoreHandle_t _connectionSlots;
  std::list<BLEControllerInternal> _controllers;
  ClientCallbacks _clientCallbacks;
};
