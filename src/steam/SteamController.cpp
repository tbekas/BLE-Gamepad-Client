#include "SteamController.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "BLEValueReceiver.h"
#include "utils.h"

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

static const std::string settingsSvcUUID = "100f6c32-1735-4313-b402-38567131e5f3";
static const std::string settingsCharUUID = "100f6c34-1735-4313-b402-38567131e5f3";

using namespace blegc;

SteamController::SteamController() {}

bool SteamController::isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  return pAdvertisedDevice->haveName() && pAdvertisedDevice->getName() == "SteamController";
}

bool SteamController::init() {
  const auto* pSettingsChar = findWritableCharacteristic(_pClient, settingsSvcUUID, settingsCharUUID);
  if (!pSettingsChar) {
    return false;
  }

  if (!pSettingsChar->writeValue(&clearMappingsCmd[0], sizeof(clearMappingsCmd))) {
    return false;
  }

  if (!pSettingsChar->writeValue(&disableLizardModeCmd[0], sizeof(disableLizardModeCmd))) {
    return false;
  }

  auto* pControlsChar = findNotifiableCharacteristic(_pClient, hidSvcUUID, inputReportChrUUID, 2);

  return BLEValueReceiver::init(pControlsChar);
}

bool SteamController::deinit() {
  return true;
}
