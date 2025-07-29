#pragma once

#include <NimBLEDevice.h>
#include <deque>
#include <list>
#include <map>
#include "BLEController.h"
#include "BLEControllerAdapter.h"
#include "BLEControllerInternal.h"

#define CTRL_ADAPTER_MATCH_TYPE uint64_t
#define MAX_CTRL_ADAPTER_COUNT sizeof(CTRL_ADAPTER_MATCH_TYPE)

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
  static void enableAutoScan();
  static void disableAutoScan();
  static bool isAutoScanEnabled();
  static bool addControllerAdapter(const BLEControllerAdapter& config);

  friend class BLEClientCallbacksImpl;
  friend class BLEScanCallbacksImpl;
  friend class BLEController;

 private:
  static BLEControllerInternal* _createController(NimBLEAddress allowedAddress);
  static BLEControllerInternal* _getController(NimBLEAddress address);
  static bool _reserveController(NimBLEAddress address);
  static bool _releaseController(NimBLEAddress address);
  static void _clientStatusConsumerFn(void* pvParameters);
  static void _autoScanCheck();
  static bool _initialized;
  static bool _autoScanEnabled;
  static QueueHandle_t _clientStatusQueue;
  static TaskHandle_t _clientStatusConsumerTask;
  static SemaphoreHandle_t _connectionSlots;
  static std::map<NimBLEAddress, CTRL_ADAPTER_MATCH_TYPE> _adapterMatch;
  static std::list<BLEControllerInternal> _controllers;
  static std::deque<BLEControllerAdapter> _adapters;
};
