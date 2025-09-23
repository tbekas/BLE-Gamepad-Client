#include "XboxController.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "../BLEValueReceiver.h"
#include "../utils.h"
#include "XboxVibrationsCommand.h"

XboxController::XboxController(const NimBLEAddress allowedAddress)
    : BLEBaseController(allowedAddress),
      _controls(XboxControlsEvent::Decoder, XboxControlsEvent::CharSpec),
      _battery(XboxBatteryEvent::Decoder, XboxBatteryEvent::CharSpec),
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

  return _controls.init(pClient) && _battery.init(pClient) && _vibrations.init(pClient);
}

/**
 * @brief Read the controls state from the connected controller.
 * @param[out] event Reference to the event instance where the data will be written.
 */
void XboxController::readControls(XboxControlsEvent& event) {
  _controls.readLast(event);
}

/**
 * @brief Sets the callback to be invoked when the controller sends update to the controls state.
 * @param callback Reference to the callback function.
 */
void XboxController::onControlsUpdate(const std::function<void(XboxControlsEvent& e)>& callback) {
  _controls.onUpdate(callback);
}

/**
 * @brief Read the battery state from the connected controller.
 * @param[out] event Reference to the event instance where the data will be written.
 */
void XboxController::readBattery(XboxBatteryEvent& event) {
  _battery.readLast(event);
}

/**
 * @brief Sets the callback to be invoked when the controller sends update to the battery state.
 * @param callback Reference to the callback function.
 */
void XboxController::onBatteryUpdate(const std::function<void(XboxBatteryEvent& e)>& callback) {
  _battery.onUpdate(callback);
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
