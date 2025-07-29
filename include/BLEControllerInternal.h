#pragma once

#include <NimBLEAddress.h>
#include "BLEBatteryEvent.h"
#include "BLEControllerAdapter.h"
#include "BLEControlsEvent.h"
#include "BLEIncomingSignal.h"
#include "BLEOutgoingSignal.h"

using OnControlsUpdate = std::function<void(BLEControlsEvent& e)>;
using OnBatteryUpdate = std::function<void(BLEBatteryEvent& e)>;
using OnConnect = std::function<void(NimBLEAddress a)>;
using OnDisconnect = std::function<void(NimBLEAddress a)>;

class BLEControllerInternal {
 public:
  explicit BLEControllerInternal(NimBLEAddress allowedAddress);
  ~BLEControllerInternal() = default;
  bool init(BLEControllerAdapter& config);
  bool deinit(bool disconnected);
  bool isInitialized() const;
  NimBLEAddress getAddress() const;
  void setAddress(NimBLEAddress address);
  NimBLEAddress getAllowedAddress() const;
  NimBLEAddress getLastAddress() const;
  void setLastAddress(NimBLEAddress address);
  void onConnect(const OnConnect& callback);
  void onDisconnect(const OnDisconnect& callback);
  BLEControlsSignal& getControls();
  BLEBatterySignal& getBattery();
  BLEVibrationsSignal& getVibrations();

 private:
  bool _initialized;
  NimBLEAddress _address;
  NimBLEAddress _allowedAddress;
  NimBLEAddress _lastAddress;
  OnConnect _onConnect{};
  OnDisconnect _onDisconnect{};
  BLEControlsSignal _controls{};
  BLEBatterySignal _battery{};
  BLEVibrationsSignal _vibrations{};
};
