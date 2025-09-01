#include "BLEXboxController.h"
#include <NimBLEAddress.h>
#include <NimBLEDevice.h>
#include "BLEIncomingSignal.h"
#include "logger.h"
#include "utils.h"
#include "xbox.h"

using namespace blegc::xbox;

BLEXboxController::BLEXboxController(const NimBLEAddress allowedAddress)
    : BLEBaseController(allowedAddress),
  _controls(controlsDecoder, controlsCharacteristic),
  _battery(batteryDecoder, batteryCharacteristic),
  _vibrations(vibrationsEncoder, vibrationsCharacteristic, vibrationsBufferLen)
  {}

BLEXboxController::BLEXboxController() : BLEXboxController(NimBLEAddress()) {}

bool BLEXboxController::isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  return false;
}

bool BLEXboxController::deinit() {
  return true;
}

bool BLEXboxController::init(NimBLEClient* pClient) {
  if (!blegc::discoverAttributes(pClient)) {
    return false;
  }

  return _controls.init(pClient) && _battery.init(pClient) && _vibrations.init(pClient);
}

/**
 * @brief Read the controls state from the connected controller.
 * @param[out] event Reference to the event instance where the data will be written.
 */
void BLEXboxController::readControls(BLEControlsEvent& event) {
  _controls.read(event);
}

/**
 * @brief Sets the callback to be invoked when the controller sends update to the controls state.
 * @param callback Reference to the callback function.
 */
void BLEXboxController::onControlsUpdate(const OnControlsUpdate& callback) {
  _controls.onUpdate(callback);
}

/**
 * @brief Read the battery state from the connected controller.
 * @param[out] event Reference to the event instance where the data will be written.
 */
void BLEXboxController::readBattery(BLEBatteryEvent& event) {
  _battery.read(event);
}

/**
 * @brief Sets the callback to be invoked when the controller sends update to the battery state.
 * @param callback Reference to the callback function.
 */
void BLEXboxController::onBatteryUpdate(const OnBatteryUpdate& callback) {
  _battery.onUpdate(callback);
}

/**
 * @brief Send the vibrations command to the connected controller.
 * @param cmd Command enabling specific motors in the controller.
 */
void BLEXboxController::writeVibrations(const BLEVibrationsCommand& cmd) {
  _vibrations.write(cmd);
}
