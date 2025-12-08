#pragma once

#include <NimBLEDevice.h>
#include <atomic>
#include <memory>
#include "utils.h"

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

  const static NimBLEAddress _nullAddress;

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
  bool hidInit(NimBLEClient* pClient);

  virtual void callOnConnecting() = 0;
  virtual void callOnConnectionFailed() = 0;
  virtual void callOnConnected() = 0;
  virtual void callOnDisconnected() = 0;
  virtual bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) = 0;
  virtual bool init(NimBLEClient* pClient) = 0;
  virtual bool deinit() = 0;

  std::atomic_bool _pendingDeregistration;
  std::atomic<const NimBLEAddress*> _address;
  NimBLEClient* _pClient;

  ConnectionState _connectionState;
  NimBLEAddress _lastAddress;
  blegc::BLEDeviceInfo _deviceInfo;
};
