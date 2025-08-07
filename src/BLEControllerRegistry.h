#pragma once

#include <NimBLEDevice.h>

#include <list>

#include "BLEController.h"
#include "BLEControllerAdapter.h"
#include "BLEControllerInternal.h"
#include "BLEGamepadClient.h"

enum BLEClientStatusMsgKind : uint8_t {
 BLEClientConnected = 0,
 BLEClientDisconnected = 1
};

struct BLEClientStatus {
 NimBLEAddress address;
 BLEClientStatusMsgKind kind;

 explicit operator std::string() const;
};

class BLEControllerRegistry {
 public:
  static bool init();
  static bool deinit();
  static bool isInitialized();

  friend class BLEClientCallbacksImpl;
  friend class BLEScanCallbacksImpl;
  friend class BLEController;
  friend class BLEGamepadClient;

  static BLEControllerInternal* createController(NimBLEAddress allowedAddress);
  static bool reserveController(NimBLEAddress address);
  static bool releaseController(NimBLEAddress address);
 private:
  static BLEControllerInternal* _getController(NimBLEAddress address);
  static void _clientStatusConsumerFn(void* pvParameters);
  static void _autoScanCheck();
  static bool _initialized;
  static bool _autoScanEnabled;
  static bool _deleteBonds;
  static QueueHandle_t _clientStatusQueue;
  static TaskHandle_t _clientStatusConsumerTask;
  static SemaphoreHandle_t _connectionSlots;
  static std::list<BLEControllerInternal> _controllers;

};
