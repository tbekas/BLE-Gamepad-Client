#pragma once

#include <NimBLEAddress.h>
#include "BatteryEvent.h"
#include "ControllerInternal.h"
#include "ControlsEvent.h"
#include "VibrationsCommand.h"

class Controller {
 public:
  Controller();
  explicit Controller(NimBLEAddress address);
  explicit Controller(const std::string& address);
  ~Controller() = default;
  bool begin();
  bool isConnected() const;
  NimBLEAddress getAddress() const;
  void onConnect(const OnConnect& callback);
  void onDisconnect(const OnDisconnect& callback);
  void readControls(ControlsEvent& event) const;
  void onControlsUpdate(const OnControlsUpdate& callback);
  void readBattery(BatteryEvent& event) const;
  void onBatteryUpdate(const OnBatteryUpdate& callback);
  void writeVibrations(const VibrationsCommand& cmd) const;

 private:
  ControllerInternal* _pCtrl;
  NimBLEAddress _allowedAddress;
  OnConnect _onConnect;
  OnDisconnect _onDisconnect;
  OnControlsUpdate _onControlsUpdate;
  bool _onControlsUpdateIsSet;
  OnBatteryUpdate _onBatteryUpdate;
  bool _onBatteryUpdateIsSet;
};
