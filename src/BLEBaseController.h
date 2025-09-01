#pragma once

#include <NimBLEDevice.h>

using OnConnect = std::function<void(NimBLEAddress a)>;
using OnDisconnect = std::function<void(NimBLEAddress a)>;

class BLEBaseController {
 public:
  virtual ~BLEBaseController() = default;
  explicit BLEBaseController(NimBLEAddress allowedAddress);

  bool begin(); // user facing
  bool end();

  NimBLEAddress getAddress() const; // user facing

 // internal
  void setAddress(NimBLEAddress address);
  NimBLEAddress getAllowedAddress() const;
  NimBLEAddress getLastAddress() const;
  void setLastAddress(NimBLEAddress address);

  bool isConnected() const; // user facing
  void setConnected(); // internal
  void setDisconnected(); // internal
  void onConnect(const OnConnect& onConnect); // user facing
  void onDisconnect(const OnDisconnect& onDisconnect); // user facing

  // internal
  virtual bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) = 0;
  virtual bool init(NimBLEClient* pClient) = 0;
  virtual bool deinit() = 0;

 protected:
  bool _connected;
  NimBLEAddress _address;
  NimBLEAddress _allowedAddress;
  NimBLEAddress _lastAddress;
  OnConnect _onConnect;
  OnDisconnect _onDisconnect;
};
