#pragma once

#include <NimBLEAddress.h>
#include "BLEBatteryEvent.h"
#include "BLEControllerInternal.h"
#include "BLEControlsEvent.h"
#include "BLEVibrationsCommand.h"

class BLEController {
 public:
  BLEController();
  explicit BLEController(NimBLEAddress address);
  explicit BLEController(const std::string& address);
  ~BLEController() = default;
  bool begin();
  bool isConnected() const;
  NimBLEAddress getAddress() const;
  void onConnect(const OnConnect& callback);
  void onDisconnect(const OnDisconnect& callback);
  void readControls(BLEControlsEvent& event) const;
  void onControlsUpdate(const OnControlsUpdate& callback);
  void readBattery(BLEBatteryEvent& event) const;
  void onBatteryUpdate(const OnBatteryUpdate& callback);
  void writeVibrations(const BLEVibrationsCommand& cmd) const;

 private:
  BLEControllerInternal* _pCtrl;
  NimBLEAddress _allowedAddress;
  OnConnect _onConnect;
  OnDisconnect _onDisconnect;
  OnControlsUpdate _onControlsUpdate;
  bool _onControlsUpdateIsSet;
  OnBatteryUpdate _onBatteryUpdate;
  bool _onBatteryUpdateIsSet;
};
