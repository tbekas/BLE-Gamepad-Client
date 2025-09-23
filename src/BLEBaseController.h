#pragma once

#include <NimBLEDevice.h>

class BLEBaseController;

class BLEBaseController {
 public:
  virtual ~BLEBaseController() = default;
  explicit BLEBaseController(NimBLEAddress allowedAddress);

  void begin();
  void end();
  NimBLEAddress getAddress() const;
  NimBLEAddress getLastAddress() const;
  NimBLEAddress getAllowedAddress() const;
  bool isConnected() const;
  void disconnect();

 friend class BLEControllerRegistry;

 protected:
  void setAddress(NimBLEAddress address);
  void setLastAddress(NimBLEAddress address);
  bool isAllocated() const;
  void markPendingDeregistration();
  void markCompletedDeregistration();
  void markConnected();
  void markDisconnected();
  bool isPendingDeregistration() const;
  void callOnConnect() const;
  void callOnDisconnect() const;
  NimBLEClient* getClient() const;

  virtual bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) = 0;
  virtual bool init(NimBLEClient* pClient) = 0;
  virtual bool deinit() = 0;

  volatile bool _pendingDeregistration;
  bool _connected;
  NimBLEAddress _address;
  NimBLEAddress _allowedAddress;
  NimBLEAddress _lastAddress;
  std::function<void()> _onConnect;
  std::function<void()> _onDisconnect;
};
