#include "SteamController.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "../BLEValueReceiver.h"
#include "../utils.h"

static auto* LOG_TAG = "SteamController";

// This is a small subset of available commands and register ids. Check the Linux driver for more
// https://github.com/torvalds/linux/blob/master/drivers/hid/hid-steam.c
#define CMD_CLEAR_MAPPINGS 0x81
#define CMD_WRITE_REGISTER 0x87
#define ID_LPAD_MODE 0x07
#define ID_RPAD_MODE 0x08
#define ID_RPAD_MARGIN 0x18
#define ID_LED 0x2d
#define ID_GYRO_MODE 0x30

// clang-format off
static  constexpr uint8_t clearMappingsCmd[] = {
  0xc0, CMD_CLEAR_MAPPINGS, // Command
  0x01                      // Command Len
};
// clang-format on

// clang-format off
static constexpr uint8_t disableLizardModeCmd[] = {
  0xc0, CMD_WRITE_REGISTER,   // Command
  0x0f,                       // Command Len
  ID_GYRO_MODE,   0x00, 0x00, // Disable gyro/accel
  ID_LPAD_MODE,   0x07, 0x00, // Disable cursor
  ID_RPAD_MODE,   0x07, 0x00, // Disable mouse
  ID_RPAD_MARGIN, 0x00, 0x00, // No margin
  ID_LED,         0x64, 0x00  // Max LED brightness
};
// clang-format on

static const auto settingsCharSpec = BLECharacteristicSpec{
  .serviceUUID = NimBLEUUID("100f6c32-1735-4313-b402-38567131e5f3"),
  .characteristicUUID = NimBLEUUID("100f6c34-1735-4313-b402-38567131e5f3"),
  .properties = uint8_t{BLE_GATT_CHR_PROP_WRITE}};

SteamController::SteamController(NimBLEAddress allowedAddress)
    : BLEBaseController(allowedAddress),
      _controls(SteamControlsEvent::Decoder, SteamControlsEvent::CharSpec) {}

SteamController::SteamController(const std::string& allowedAddress)
    : SteamController(NimBLEAddress(allowedAddress, BLE_ADDR_PUBLIC)) {}

SteamController::SteamController() : SteamController(NimBLEAddress()) {}

bool SteamController::isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  return pAdvertisedDevice->haveName() && pAdvertisedDevice->getName() == "SteamController";
}
bool SteamController::init(NimBLEClient* pClient) {
  if (!blegc::discoverAttributes(pClient)) {
    return false;
  }

  const auto* pChar = blegc::findCharacteristic(pClient, settingsCharSpec);
  if (!pChar) {
    return false;
  }

  if (!pChar->canWrite()) {
    BLEGC_LOGE(LOG_TAG, "Characteristic not able to write. %s", blegc::remoteCharToStr(pChar).c_str());
    return false;
  }

  if (!pChar->writeValue(&clearMappingsCmd[0], sizeof(clearMappingsCmd))) {
    return false;
  }

  if (!pChar->writeValue(&disableLizardModeCmd[0], sizeof(disableLizardModeCmd))) {
    return false;
  }

  return _controls.init(pClient);
}
bool SteamController::deinit() {
  return true;
}

void SteamController::readControls(SteamControlsEvent& event) {
  _controls.readLast(event);
}

/**
 * @brief Sets the callback to be invoked when the controller sends update to the controls state.
 * @param callback Reference to the callback function.
 */
void SteamController::onControlsUpdate(const std::function<void(SteamControlsEvent& e)>& callback) {
  _controls.onUpdate(callback);
}

/**
 * @brief Sets the callback to be invoked when the controller connects.
 * @param callback Reference to a callback function.
 */
void SteamController::onConnect(const std::function<void(SteamController& c)>& callback) {
  _onConnect = [callback, this]() -> void { callback(*this); };
}

/**
 * @brief Sets the callback to be invoked when the controller disconnects.
 * @param callback Reference to the callback function.
 */
void SteamController::onDisconnect(const std::function<void(SteamController& c)>& callback) {
  _onConnect = [callback, this]() -> void { callback(*this); };
}
