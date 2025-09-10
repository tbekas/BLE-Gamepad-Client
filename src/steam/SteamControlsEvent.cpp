#include "SteamControlsEvent.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "../logger.h"
#include "../utils.h"

// Sources
// https://github.com/torvalds/linux/blob/master/drivers/hid/hid-steam.c
// https://github.com/ricardoquesada/bluepad32/blob/main/src/components/bluepad32/parser/uni_hid_parser_steam.c

#define REPORT_TYPE_INPUT 0x04
#define CONTAINS_BUTTONS_DATA 0x0010
#define CONTAINS_TRIGGERS_DATA 0x0020
#define CONTAINS_THUMBSTICK_DATA 0x0080
#define CONTAINS_LEFT_PAD_DATA 0x0100
#define CONTAINS_RIGHT_PAD_DATA 0x0200
#define CONTAINS_GYRO_DATA 0x1000

blegc::BLEDecodeResult decodeControlsEvent(SteamControlsEvent& e, uint8_t payload[], size_t payloadLen);

const blegc::BLEValueDecoder<SteamControlsEvent> SteamControlsEvent::Decoder(decodeControlsEvent);
const blegc::BLECharacteristicSpec SteamControlsEvent::CharSpec{
    .serviceUUID = NimBLEUUID(blegc::hidSvcUUID),
    .characteristicUUID = NimBLEUUID(blegc::inputReportChrUUID),
    .properties = BLE_GATT_CHR_PROP_NOTIFY,
    .idx = 2};

constexpr size_t controlsPayloadLen = 19;

inline uint16_t make_uint16(const uint8_t lsb, const uint8_t msb) {
  uint16_t val = msb;
  val <<= 8;
  val += lsb;
  return val;
}

inline int16_t make_int16(const uint8_t lsb, const uint8_t msb) {
  int16_t val = msb;
  val <<= 8;
  val += lsb;
  return val;
}

inline bool decodeButton(uint8_t byte, int bit) {
  return byte & 1 << bit;
}

inline float decodePad(const uint8_t lsb, const uint8_t msb) {
  return static_cast<float>(make_int16(lsb, msb)) / INT16_MAX;
}

inline float decodeTrigger(const uint8_t b) {
  return static_cast<float>(b) / UINT8_MAX;
}

blegc::BLEDecodeResult decodeControlsEvent(SteamControlsEvent& e, uint8_t payload[], size_t payloadLen) {
  if (payloadLen != controlsPayloadLen) {
    return blegc::BLEDecodeResult::InvalidReport;
  }

  if (payload[0] != 0xc0) {
    return blegc::BLEDecodeResult::NotSupported;
  }

  if (payload[1] & 0x0f != REPORT_TYPE_INPUT) {
    return blegc::BLEDecodeResult::NotSupported;
  }

  const auto contentInfo = make_uint16(payload[1], payload[2]);

  auto offset = 3;
  if (contentInfo & CONTAINS_BUTTONS_DATA) {
    const uint8_t byte0 = payload[offset];
    e.rightTriggerButton = decodeButton(byte0, 0);
    e.leftTriggerButton = decodeButton(byte0, 1);
    e.rightBumper = decodeButton(byte0, 2);
    e.leftBumper = decodeButton(byte0, 3);
    e.buttonY = decodeButton(byte0, 4);
    e.buttonB = decodeButton(byte0, 5);
    e.buttonX = decodeButton(byte0, 6);
    e.buttonA = decodeButton(byte0, 7);

    const uint8_t byte1 = payload[offset + 1];

    e.dpadUp = decodeButton(byte1, 0);
    e.dpadRight = decodeButton(byte1, 1);
    e.dpadLeft = decodeButton(byte1, 2);
    e.dpadDown = decodeButton(byte1, 3);
    e.selectButton = decodeButton(byte1, 4);
    e.steamButton = decodeButton(byte1, 5);
    e.startButton = decodeButton(byte1, 6);
    e.leftGripButton = decodeButton(byte1, 7);

    const uint8_t byte2 = payload[offset + 2];
    e.rightGripButton = decodeButton(byte2, 0);
    e.leftPadClick = decodeButton(byte2, 1);
    e.rightPadClick = decodeButton(byte2, 2);
    e.leftPadTouch = decodeButton(byte2, 3);
    e.rightPadTouch = decodeButton(byte2, 4);
    e.stickButton = decodeButton(byte2, 6);
    offset += 3;
  }

  if (contentInfo & CONTAINS_TRIGGERS_DATA) {
    e.leftTrigger = decodeTrigger(payload[offset]);
    e.rightTrigger = decodeTrigger(payload[offset + 1]);
    offset += 2;
  }

  if (contentInfo & CONTAINS_THUMBSTICK_DATA) {
    e.stickX = decodePad(payload[offset], payload[offset + 1]);
    e.stickY = decodePad(payload[offset + 2], payload[offset + 3]);
    offset += 4;
  }

  if (contentInfo & CONTAINS_LEFT_PAD_DATA) {
    e.leftPadX = decodePad(payload[offset], payload[offset + 1]);
    e.leftPadY = decodePad(payload[offset + 2], payload[offset + 3]);
    offset += 4;
  }

  if (contentInfo & CONTAINS_RIGHT_PAD_DATA) {
    e.rightPadX = decodePad(payload[offset], payload[offset + 1]);
    e.rightPadY = decodePad(payload[offset + 2], payload[offset + 3]);
    offset += 4;
  }

  return blegc::BLEDecodeResult::Success;
}
