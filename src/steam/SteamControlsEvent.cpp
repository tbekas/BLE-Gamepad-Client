#include "SteamControlsEvent.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "../logger.h"
#include "../utils.h"

static auto* LOG_TAG = "XboxControlsEvent";

size_t decodeControlsEvent(SteamControlsEvent& e, uint8_t payload[], size_t payloadLen);

const blegc::BLEValueDecoder<SteamControlsEvent> SteamControlsEvent::Decoder(decodeControlsEvent);
const blegc::BLECharacteristicLocation SteamControlsEvent::CharacteristicLocation{
  .serviceUUID = NimBLEUUID("100f6c32-1735-4313-b402-38567131e5f3"),
  .characteristicUUID = NimBLEUUID("100f6c33-1735-4313-b402-38567131e5f3"),
  .properties = uint8_t{BLE_GATT_CHR_PROP_NOTIFY}};

constexpr size_t controlsPayloadLen = 12345;

void printBits(uint8_t value) {
  for (int i = 7; i >= 0; --i) {
    printf("%d", (value >> i) & 1);
  }
  printf("\n");
}

inline uint16_t uint16(uint8_t r, uint8_t l) {
  uint16_t val = l;
  val <<= 8;
  val += r;
  return val;
}

constexpr uint8_t mask(int bit) {
  return uint8_t(1) << bit;
}

inline bool decodeButton(uint8_t byte, int bit) {
  return byte & mask(bit);
}

size_t decodeControlsEvent(SteamControlsEvent& e, uint8_t payload[], size_t payloadLen) {
  BLEGC_LOGD(LOG_TAG, "Received %d bytes", payloadLen);

  for (size_t j = 0; j < payloadLen; ++j) {
    uint8_t value = payload[j];
    std::string valueStr;
    for (int i = 7; i >= 0; --i) {
      valueStr += ((value >> i) & 1) ? '1' : '0';
    }
    BLEGC_LOGD(LOG_TAG, "%s", valueStr.c_str());
  }

  if (payloadLen != controlsPayloadLen) {
    BLEGC_LOGE(LOG_TAG, "Expected %d bytes, was %d bytes", controlsPayloadLen, payloadLen);
    return 0;
  }

  if (false) {
    uint8_t byte5 = payload[5];
    // e.rightTriggerButton = decodeButton(byte5, 0);
    // e.leftTriggerButton = decodeButton(byte5, 1);
    e.buttonY = decodeButton(byte5, 4);
    e.buttonB = decodeButton(byte5, 5);
    e.buttonX = decodeButton(byte5, 6);
    e.buttonA = decodeButton(byte5, 7);

    uint8_t byte6 = payload[6];
    e.dpadUp = decodeButton(byte6, 0);
    e.dpadRight = decodeButton(byte6, 1);
    e.dpadLeft = decodeButton(byte6, 1);
    e.dpadDown = decodeButton(byte6, 1);

    // e.forward =
    // e.back =
    // e.steam =

    uint8_t byte8 = payload[8];
    // e.touchLeftPad = decodeButton(byte8, 4);
    // e.touchRightPad = decodeButton(byte8, 5);
    e.leftStickButton = decodeButton(byte8, 6);
  }


  return controlsPayloadLen;
}
