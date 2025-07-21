#pragma once

#include <NimBLEDevice.h>
#include <deque>
#include <list>
#include <map>
#include "Controller.h"
#include "ControllerConfig.h"

typedef Controller* ControllerPtr;
typedef std::function<void(Controller& controller)> ControllerCallback;

class BLEGamepadClient_ {
 public:
  static BLEGamepadClient_& getInstance();

  bool begin(bool autoScanEnabled = true, int maxConnected = 1, bool deleteBonds = false);
  bool end();

  std::list<Controller>& getControllers();
  ControllerPtr getControllerPtrByAddress(NimBLEAddress address);
  void setConnectedCallback(const ControllerCallback& onConnected);
  void setDisconnectedCallback(const ControllerCallback& onDisconnected);
  bool addConfig(const ControllerConfig& config);

  /* proxy methods below */
  bool startScan(uint32_t durationMs);
  bool stopScan();


  friend class ClientCallbacks;
  friend class ScanCallbacks;

 private:
  BLEGamepadClient_();

  static void _clientStatusConsumerFn(void* pvParameters);
  Controller& _getOrCreateController(NimBLEAddress address);
  void _autoScanCheck();
  bool _initialized;
  bool _autoScanEnabled;
  int _maxConnected;
  QueueHandle_t _clientStatusQueue;
  TaskHandle_t _clientStatusConsumerTask;
  SemaphoreHandle_t _connectionSlots;
  std::map<NimBLEAddress, uint64_t> _configMatch;
  std::list<Controller> _controllers;
  ControllerCallback _onConnected;
  ControllerCallback _onDisconnected;
  std::deque<ControllerConfig> _configs;
};

extern BLEGamepadClient_& BLEGamepadClient;
