#pragma once

#include <NimBLEDevice.h>
#include <deque>
#include <list>
#include <map>
#include "Controller.h"
#include "ControllerInternal.h"
#include "ControllerConfig.h"

class Controller;

class BLEGamepadClient {
 public:
  static bool init();
  static bool deinit();
  static bool isInitialized();
  static void disableAutoScan();
  static bool addConfig(const ControllerConfig& config);

  friend class ClientCallbacks;
  friend class ScanCallbacks;

  // TODO: package-protected
  static ControllerInternal* createController(NimBLEAddress allowedAddress);

 private:
  static void _clientStatusConsumerFn(void* pvParameters);

  static bool _releaseController(NimBLEAddress address);
  static bool _reserveController(NimBLEAddress address);
  static ControllerInternal* _getController(NimBLEAddress address);
  static void _autoScanCheck();
  static bool _initialized;
  static bool _autoScanEnabled;
  static QueueHandle_t _clientStatusQueue;
  static TaskHandle_t _clientStatusConsumerTask;
  static SemaphoreHandle_t _connectionSlots;
  static std::map<NimBLEAddress, uint64_t> _configMatch;
  static std::list<ControllerInternal> _controllers;
  static std::deque<ControllerConfig> _configs;
};
