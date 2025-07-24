#pragma once

#include <NimBLEDevice.h>
#include <deque>
#include <list>
#include <map>
#include "Controller.h"
#include "ControllerConfig.h"

typedef ControllerInternal* ControllerPtr;
typedef std::function<void(ControllerInternal& controller)> ControllerCallback;

class BLEGamepadClient_ {
 public:
  static BLEGamepadClient_& getInstance();

  bool init();
  bool end();

  std::list<ControllerInternal>& getControllers();
  ControllerPtr getControllerPtrByAddress(NimBLEAddress address);
  void setConnectedCallback(const ControllerCallback& onConnected);
  void setDisconnectedCallback(const ControllerCallback& onDisconnected);
  bool addConfig(const ControllerConfig& config);

  /* proxy methods below */
  bool startScan(uint32_t durationMs);
  bool stopScan();

  bool isInitialized() const;


  friend class ClientCallbacks;
  friend class ScanCallbacks;

 // TODO: package-protected
 ControllerInternal* createController(NimBLEAddress allowedAddress);

 private:
  BLEGamepadClient_();

  bool _releaseController(NimBLEAddress address);
  bool _reserveController(NimBLEAddress address);

  static void _clientStatusConsumerFn(void* pvParameters);
  ControllerInternal* _getController(NimBLEAddress address);
  void _autoScanCheck();
  bool _initialized;
  bool _autoScanEnabled;
  QueueHandle_t _clientStatusQueue;
  TaskHandle_t _clientStatusConsumerTask;
  SemaphoreHandle_t _connectionSlots;
  std::map<NimBLEAddress, uint64_t> _configMatch;
  std::list<ControllerInternal> _controllers;
  std::deque<ControllerConfig> _configs;
};

extern BLEGamepadClient_& BLEGamepadClient;
