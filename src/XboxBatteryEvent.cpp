#include "XboxBatteryEvent.h"

#include <NimBLEDevice.h>
#include "logger.h"
#include "utils.h"

static auto* LOG_TAG = "XboxControlsEvent";

constexpr size_t batteryPayloadLen = 1;

size_t decodeBatteryEvent(XboxBatteryEvent& e, uint8_t payload[], size_t payloadLen) {
  if (payloadLen != batteryPayloadLen) {
    BLEGC_LOGE(LOG_TAG, "Expected %d bytes, was %d bytes", batteryPayloadLen, payloadLen);
    return 0;
  }

  e.level = 0.01f * static_cast<float>(payload[0]);
  return batteryPayloadLen;
}

const blegc::BLEValueDecoder<XboxBatteryEvent> XboxBatteryEvent::Decoder(decodeBatteryEvent);
const blegc::BLECharacteristicLocation XboxBatteryEvent::CharacteristicLocation{
    .serviceUUID = NimBLEUUID(uint16_t{0x180f}),
    .characteristicUUID = NimBLEUUID(uint16_t{0x2a19}),
    .properties = uint8_t{BLE_GATT_CHR_PROP_NOTIFY}};
