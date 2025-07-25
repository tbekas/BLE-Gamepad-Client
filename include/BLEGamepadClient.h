#pragma once

#include <NimBLEDevice.h>
#include <deque>
#include <list>
#include <map>
#include "Controller.h"
#include "ControllerConfig.h"
#include "ControllerInternal.h"

#define CTRL_CONFIG_MATCH_TYPE uint64_t
#define MAX_CTRL_CONFIG_COUNT sizeof(CTRL_CONFIG_MATCH_TYPE)

enum BLEClientStatusMsgKind : uint8_t {
 BLEClientConnected = 0,
 BLEClientDisconnected = 1
};

struct BLEClientStatus {
 NimBLEAddress address;
 BLEClientStatusMsgKind kind;

 explicit operator std::string() const;
};

class BLEGamepadClient {
 public:
  static bool init();
  static bool deinit();
  static bool isInitialized();
  static bool addControllerConfig(const ControllerConfig& config);

  friend class ClientCallbacks;
  friend class ScanCallbacks;
  friend class Controller;

 private:
  static ControllerInternal* _createController(NimBLEAddress allowedAddress);
  static ControllerInternal* _getController(NimBLEAddress address);
  static bool _reserveController(NimBLEAddress address);
  static bool _releaseController(NimBLEAddress address);
  static void _clientStatusConsumerFn(void* pvParameters);
  static void _autoScanCheck();
  static bool _initialized;
  static QueueHandle_t _clientStatusQueue;
  static TaskHandle_t _clientStatusConsumerTask;
  static SemaphoreHandle_t _connectionSlots;
  static std::map<NimBLEAddress, CTRL_CONFIG_MATCH_TYPE> _configMatch;
  static std::list<ControllerInternal> _controllers;
  static std::deque<ControllerConfig> _configs;
};
