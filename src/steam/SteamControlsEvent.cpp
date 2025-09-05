#include "SteamControlsEvent.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "../logger.h"
#include "../utils.h"

static auto* LOG_TAG = "XboxControlsEvent";

#define INPUT_REPORT 0x04

#define CONTAINS_BUTTONS_DATA 0x0010
#define CONTAINS_TRIGGERS_DATA 0x0020
#define CONTAINS_THUMBSTICK_DATA 0x0080
#define CONTAINS_LEFT_PAD_DATA 0x0100
#define CONTAINS_RIGHT_PAD_DATA 0x0200

size_t decodeControlsEvent(SteamControlsEvent& e, uint8_t payload[], size_t payloadLen);

const blegc::BLEValueDecoder<SteamControlsEvent> SteamControlsEvent::Decoder(decodeControlsEvent);
const blegc::BLECharacteristicLocation SteamControlsEvent::CharacteristicLocation{
    .serviceUUID = NimBLEUUID(uint16_t{0x1812}),
    .characteristicUUID = NimBLEUUID(uint16_t{0x2a4d}),
    .properties = uint8_t{BLE_GATT_CHR_PROP_NOTIFY},
    .idx = 2};

// const blegc::BLECharacteristicLocation SteamControlsEvent::CharacteristicLocation{
//   .serviceUUID = NimBLEUUID("100f6c32-1735-4313-b402-38567131e5f3"),
//   .characteristicUUID = NimBLEUUID("100f6c33-1735-4313-b402-38567131e5f3"),
//   .properties = uint8_t{BLE_GATT_CHR_PROP_NOTIFY}};

constexpr size_t controlsPayloadLen = 19;

inline uint16_t make_uint16(uint8_t r, uint8_t l) {
  uint16_t val = l;
  val <<= 8;
  val += r;
  return val;
}

inline int16_t make_int16(const uint8_t lsb, const uint8_t msb) {
  int16_t val = (lsb | msb << 8);
  return val;
}

constexpr uint8_t mask(int bit) {
  return static_cast<uint8_t>(1) << bit;
}

inline bool decodeButton(uint8_t byte, int bit) {
  return byte & mask(bit);
}

// TODO use pointers?
inline float decodePad(const uint8_t lsb, const uint8_t msb) {
  return static_cast<float>(make_int16(lsb, msb)) / INT16_MAX;
}

inline float decodeTrigger(const uint8_t b) {
  return static_cast<float>(b) / UINT8_MAX;
}

size_t decodeControlsEvent(SteamControlsEvent& e, uint8_t payload[], size_t payloadLen) {

  if (payloadLen != controlsPayloadLen) {
    BLEGC_LOGE(LOG_TAG, "Expected %d bytes, was %d bytes", controlsPayloadLen, payloadLen);
    return 0;
  }

  if ((payload[1] & 0x0f) != INPUT_REPORT) {
    // BLEGC_LOGE(LOG_TAG, "Unexpected report type 0x%02x", payload[1] & 0x0f);
    // return 0;
    return 1;
  }

  const auto contentInfo = make_uint16(payload[1] & 0xf0, payload[2]);

  auto offset = 3;

  if (contentInfo & CONTAINS_BUTTONS_DATA) {
    // TODO remove this - debug data
    size_t copyLen = min(sizeof(e.data), payloadLen);
    memcpy(e.data, payload, copyLen);
    e.dataLen = copyLen;

    const uint8_t byte3 = payload[offset];
    e.rightTriggerButton = decodeButton(byte3, 0);
    e.leftTriggerButton = decodeButton(byte3, 1);
    e.rightBumper = decodeButton(byte3, 2);
    e.leftBumper = decodeButton(byte3, 3);
    e.buttonY = decodeButton(byte3, 4);
    e.buttonB = decodeButton(byte3, 5);
    e.buttonX = decodeButton(byte3, 6);
    e.buttonA = decodeButton(byte3, 7);

    const uint8_t byte4 = payload[offset + 1];

    e.dpadUp = decodeButton(byte4, 0);
    e.dpadRight = decodeButton(byte4, 1);
    e.dpadLeft = decodeButton(byte4, 2);
    e.dpadDown = decodeButton(byte4, 3);
    e.selectButton = decodeButton(byte4, 4);
    e.systemButton = decodeButton(byte4, 5);
    e.startButton = decodeButton(byte4, 6);
    e.leftGripButton = decodeButton(byte4, 7);

    const uint8_t byte5 = payload[offset + 2];
    e.rightGripButton = decodeButton(byte5, 0);
    e.leftPadClick = decodeButton(byte5, 1);
    e.rightPadClick = decodeButton(byte5, 2);
    e.leftPadTouch = decodeButton(byte5, 3);
    e.rightPadTouch = decodeButton(byte5, 4);
    e.stickButton = decodeButton(byte5, 6);
  }

  if (contentInfo & CONTAINS_TRIGGERS_DATA) {
    e.leftTrigger = decodeTrigger(payload[offset]);
    e.rightTrigger = decodeTrigger(payload[offset + 1]);
  }

  if (contentInfo & CONTAINS_THUMBSTICK_DATA) {
    e.stickX = decodePad(payload[offset], payload[offset + 1]);
    e.stickY = decodePad(payload[offset + 2], payload[offset + 3]);
  }

  if (contentInfo & CONTAINS_LEFT_PAD_DATA) {
    e.leftPadX = decodePad(payload[offset], payload[offset + 1]);
    e.leftPadY = decodePad(payload[offset + 2], payload[offset + 3]);
    offset += 4;
  }

  if (contentInfo & CONTAINS_RIGHT_PAD_DATA) {
    e.rightPadX = decodePad(payload[offset], payload[offset + 1]);
    e.rightPadY = decodePad(payload[offset + 2], payload[offset + 3]);
  }

  return 1;
}
