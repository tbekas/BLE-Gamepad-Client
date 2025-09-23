#include "XboxController.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "../BLEValueReceiver.h"
#include "../utils.h"
#include "XboxVibrationsCommand.h"

XboxController::XboxController(const NimBLEAddress allowedAddress)
    : BLEBaseController(allowedAddress),
      BLEControlsAPI(XboxControlsEvent::Decoder, XboxControlsEvent::CharSpec),
      BLEBatteryAPI(XboxBatteryEvent::Decoder, XboxBatteryEvent::CharSpec),
      _vibrations(XboxVibrationsCommand::Encoder, XboxVibrationsCommand::CharSpec) {}

XboxController::XboxController(const std::string& allowedAddress)
    : XboxController(NimBLEAddress(allowedAddress, BLE_ADDR_PUBLIC)) {}

XboxController::XboxController() : XboxController(NimBLEAddress()) {}

bool XboxController::isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  return pAdvertisedDevice->haveName() && pAdvertisedDevice->getName() == "Xbox Wireless Controller";
}

bool XboxController::deinit() {
  return true;
}

bool XboxController::init(NimBLEClient* pClient) {
  if (!blegc::discoverAttributes(pClient)) {
    return false;
  }

  return BLEControlsAPI::init(pClient) && BLEBatteryAPI::init(pClient) && _vibrations.init(pClient);
}


/**
 * @brief Send the vibrations command to the connected controller.
 * @param cmd Command enabling specific motors in the controller.
 */
void XboxController::writeVibrations(const XboxVibrationsCommand& cmd) {
  _vibrations.write(cmd);
}

/**
 * @brief Sets the callback to be invoked when the controller connects.
 * @param callback Reference to a callback function.
 */
void XboxController::onConnect(const std::function<void(XboxController& c)>& callback) {
  _onConnect = [callback, this]() -> void { callback(*this); };
}

/**
 * @brief Sets the callback to be invoked when the controller disconnects.
 * @param callback Reference to the callback function.
 */
void XboxController::onDisconnect(const std::function<void(XboxController& c)>& callback) {
  _onDisconnect = [callback, this]() -> void { callback(*this); };
}
