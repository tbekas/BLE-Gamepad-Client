#include "SteamController.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "../BLENotifiableSignal.h"
#include "../utils.h"

static auto* LOG_TAG = "SteamController";

// Commands that can be sent in a feature report.
#define STEAM_CMD_SET_MAPPINGS 0x80
#define STEAM_CMD_CLEAR_MAPPINGS 0x81
#define STEAM_CMD_GET_MAPPINGS 0x82
#define STEAM_CMD_GET_ATTRIB 0x83
#define STEAM_CMD_GET_ATTRIB_LABEL 0x84
#define STEAM_CMD_DEFAULT_MAPPINGS 0x85
#define STEAM_CMD_FACTORY_RESET 0x86
#define STEAM_CMD_WRITE_REGISTER 0x87

// Some useful register ids
#define STEAM_REG_LPAD_MODE 0x07
#define STEAM_REG_RPAD_MODE 0x08
#define STEAM_REG_RPAD_MARGIN 0x18
#define STEAM_REG_LED 0x2d
#define STEAM_REG_GYRO_MODE 0x30
#define STEAM_REG_LPAD_CLICK_PRESSURE 0x34
#define STEAM_REG_RPAD_CLICK_PRESSURE 0x35

static uint8_t cmd_clear_mappings[] = {
  0xc0, STEAM_CMD_CLEAR_MAPPINGS,  // Command
  0x01                             // Command Len
};


static uint8_t cmd_disable_lizard[] = {
  0xc0, STEAM_CMD_WRITE_REGISTER,    // Command
  0x0f,                              // Command Len
  STEAM_REG_GYRO_MODE,   0x00, 0x00, // Disable gyro/accel
  STEAM_REG_LPAD_MODE,   0x07, 0x00, // Disable cursor
  STEAM_REG_RPAD_MODE,   0x07, 0x00, // Disable mouse
  STEAM_REG_RPAD_MARGIN, 0x00, 0x00, // No margin
  STEAM_REG_LED,         0x64, 0x00  // LED bright, max value
};

SteamController::SteamController(NimBLEAddress allowedAddress)
    : BLEBaseController(allowedAddress),
      _controls(SteamControlsEvent::Decoder, SteamControlsEvent::CharacteristicLocation) {}

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

  const auto location = blegc::BLECharacteristicLocation{
    .serviceUUID = NimBLEUUID("100f6c32-1735-4313-b402-38567131e5f3"),
    .characteristicUUID = NimBLEUUID("100f6c34-1735-4313-b402-38567131e5f3"),
    .properties = uint8_t{BLE_GATT_CHR_PROP_WRITE}};

  auto* pChar = blegc::findCharacteristic(pClient, location);
  if (!pChar) {
    return false;
  }

  if (!pChar->canWrite()) {
    BLEGC_LOGE(LOG_TAG, "Characteristic not able to write. %s", blegc::remoteCharToStr(pChar).c_str());
    return false;
  }

  if (!pChar->writeValue(&cmd_clear_mappings[0], sizeof(cmd_clear_mappings))) {
    return false;
  }

  if (!pChar->writeValue(&cmd_disable_lizard[0], sizeof(cmd_disable_lizard))) {
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
