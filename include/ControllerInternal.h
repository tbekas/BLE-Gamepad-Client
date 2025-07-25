#pragma once

#include <NimBLEAddress.h>
#include "BatteryEvent.h"
#include "ControllerConfig.h"
#include "ControlsEvent.h"
#include "IncomingSignal.h"
#include "OutgoingSignal.h"

typedef std::function<void(ControlsEvent& e)> OnControlsUpdate;
typedef std::function<void(BatteryEvent& e)> OnBatteryUpdate;
typedef std::function<void(NimBLEAddress a)> OnConnect;
typedef std::function<void(NimBLEAddress a)> OnDisconnect;

class ControllerInternal {
public:
  explicit ControllerInternal(NimBLEAddress allowedAddress);
  ~ControllerInternal() = default;

  bool isInitialized() const;

  NimBLEAddress getAddress() const;
  void setAddress(NimBLEAddress address);

  NimBLEAddress getLastAddress() const;
  void setLastAddress(NimBLEAddress address);

  NimBLEAddress getAllowedAddress() const;

  void onConnect(const OnConnect& callback);
  void onDisconnect(const OnDisconnect& callback);

  ControlsSignal& getControls();
  BatterySignal& getBattery();
  VibrationsSignal& getVibrations();

  bool init(ControllerConfig& config);
  bool deinit(bool disconnected);

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
