#pragma once

#include <NimBLEAddress.h>
#include "BatteryEvent.h"
#include "ControllerConfig.h"
#include "ControlsEvent.h"
#include "IncomingSignal.h"
#include "OutgoingSignal.h"

using OnControlsUpdate = std::function<void(ControlsEvent& e)>;
using OnBatteryUpdate = std::function<void(BatteryEvent& e)>;
using OnConnect = std::function<void(NimBLEAddress a)>;
using OnDisconnect = std::function<void(NimBLEAddress a)>;

class ControllerInternal {
 public:
  explicit ControllerInternal(NimBLEAddress allowedAddress);
  ~ControllerInternal() = default;
  bool init(ControllerConfig& config);
  bool deinit(bool disconnected);
  bool isInitialized() const;
  NimBLEAddress getAddress() const;
  void setAddress(NimBLEAddress address);
  NimBLEAddress getAllowedAddress() const;
  NimBLEAddress getLastAddress() const;
  void setLastAddress(NimBLEAddress address);
  void onConnect(const OnConnect& callback);
  void onDisconnect(const OnDisconnect& callback);
  ControlsSignal& getControls();
  BatterySignal& getBattery();
  VibrationsSignal& getVibrations();

 private:
  bool _initialized;
  NimBLEAddress _address;
  NimBLEAddress _allowedAddress;
  NimBLEAddress _lastAddress;
  OnConnect _onConnect{};
  OnDisconnect _onDisconnect{};
  ControlsSignal _controls{};
  BatterySignal _battery{};
  VibrationsSignal _vibrations{};
};
