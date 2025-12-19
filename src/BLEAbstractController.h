#pragma once

#include <NimBLEDevice.h>
#include <atomic>
#include <memory>
#include "BLEDeviceInfo.h"

class BLEAbstractController {
 public:
  virtual ~BLEAbstractController() = default;
  explicit BLEAbstractController();

  void begin();
  void end();
  NimBLEAddress getAddress() const;
  NimBLEAddress getLastAddress() const;
  bool isConnected() const;
  bool isConnecting() const;
  void disconnect();

  friend class BLEUserCallbackRunner;
  friend class BLEControllerRegistry;

 protected:
  enum class ConnectionState : uint8_t {
    Connecting,
    Connected,
    Disconnected,
  };

  NimBLEClient* getClient() const;
  void setClient(NimBLEClient* pClient);
  bool tryAllocate(NimBLEAddress address);
  bool tryDeallocate();
  bool isAllocated() const;
  void markCompletedDeregistration();
  void markConnecting();
  void markConnected();
  void markDisconnected();
  bool isPendingDeregistration() const;
  bool hidInit();

  virtual void callOnConnecting() = 0;
  virtual void callOnConnectionFailed() = 0;
  virtual void callOnConnected() = 0;
  virtual void callOnDisconnected() = 0;
  virtual bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) = 0;
  virtual bool init() = 0;
  virtual bool deinit() = 0;

  static uint64_t _encodeAddress(const NimBLEAddress& address);
  static NimBLEAddress _decodeAddress(const uint64_t& address);
  void _readDeviceInfo();

  std::atomic_bool _pendingDeregistration;
  std::atomic_uint64_t _address;
  NimBLEClient* _pClient;

  ConnectionState _connectionState;
  NimBLEAddress _lastAddress;
  BLEDeviceInfo _deviceInfo;
};
