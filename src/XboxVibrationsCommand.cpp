#include "XboxVibrationsCommand.h"

#include <NimBLEDevice.h>
#include "logger.h"
#include "utils.h"

static auto* LOG_TAG = "XboxVibrationsCommand";

size_t encodeVibrationsCommand(const XboxVibrationsCommand& c, uint8_t outBuffer[], size_t bufferLen);

const blegc::BLEValueEncoder<XboxVibrationsCommand> XboxVibrationsCommand::Encoder(encodeVibrationsCommand);
const blegc::BLECharacteristicLocation XboxVibrationsCommand::CharacteristicLocation{
  .serviceUUID = NimBLEUUID(static_cast<uint16_t>(0x1812)),
  .characteristicUUID = NimBLEUUID(static_cast<uint16_t>(0x2a4d)),
  .properties = BLE_GATT_CHR_PROP_WRITE};

constexpr size_t vibrationsPayloadLen = 8;

inline uint16_t uint16(uint8_t r, uint8_t l) {
  uint16_t val = l;
  val <<= 8;
  val += r;
  return val;
}

constexpr uint8_t mask(int bit) {
  return uint8_t(1) << bit;
}

inline uint8_t encodeMotorEnable(float power, int bit) {
  return (power > 0.0f) * mask(bit);
}

inline uint8_t encodeMotorPower(float power) {
  return static_cast<uint8_t>(max(min(power, 1.0f), 0.0f) * 100.0f);
}

inline uint8_t encodeDuration(uint32_t durationMs) {
  return static_cast<uint8_t>(min(durationMs, static_cast<uint32_t>(2550)) / 10);
}

size_t encodeVibrationsCommand(const XboxVibrationsCommand& c, uint8_t outBuffer[], size_t bufferLen) {
  if (bufferLen < vibrationsPayloadLen) {
    BLEGC_LOGD(LOG_TAG, "Expected buffer of at least %d bytes, was %d bytes", vibrationsPayloadLen, bufferLen);
    return 0;
  }

  if (c.cycles == 0) {
    outBuffer[0] = 0;
  } else {
    outBuffer[0] = encodeMotorEnable(c.rightMotor, 0) | encodeMotorEnable(c.leftMotor, 1) |
                   encodeMotorEnable(c.rightTriggerMotor, 2) | encodeMotorEnable(c.leftTriggerMotor, 3);
  }

  outBuffer[1] = encodeMotorPower(c.leftTriggerMotor);
  outBuffer[2] = encodeMotorPower(c.rightTriggerMotor);
  outBuffer[3] = encodeMotorPower(c.leftMotor);
  outBuffer[4] = encodeMotorPower(c.rightMotor);
  outBuffer[5] = encodeDuration(c.durationMs);
  outBuffer[6] = encodeDuration(c.pauseMs);
  outBuffer[7] = c.cycles == 0 ? 0 : c.cycles - 1;

  return vibrationsPayloadLen;
}
