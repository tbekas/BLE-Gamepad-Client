#pragma once

#include <NimBLEDevice.h>

using OnConnect = std::function<void(NimBLEAddress a)>;
using OnDisconnect = std::function<void(NimBLEAddress a)>;

class BLEBaseController {
 public:
  virtual ~BLEBaseController() = default;
  explicit BLEBaseController(NimBLEAddress allowedAddress);

  void begin();
  void end();
  NimBLEAddress getAddress() const;
  NimBLEAddress getAllowedAddress() const;
  bool isConnected() const;
  void onConnect(const OnConnect& onConnect);
  void onDisconnect(const OnDisconnect& onDisconnect);
  void disconnect();

 friend class BLEControllerRegistry;

 protected:
  void setAddress(NimBLEAddress address);
  NimBLEAddress getLastAddress() const;
  void setLastAddress(NimBLEAddress address);
  void setConnected();
  void setDisconnected();

  virtual bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) = 0;
  virtual bool init(NimBLEClient* pClient) = 0;
  virtual bool deinit() = 0;

  bool _connected;
  NimBLEAddress _address;
  NimBLEAddress _allowedAddress;
  NimBLEAddress _lastAddress;
  OnConnect _onConnect;
  OnDisconnect _onDisconnect;
};
