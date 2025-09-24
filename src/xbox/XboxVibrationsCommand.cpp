#include "XboxVibrationsCommand.h"

#include <NimBLEDevice.h>
#include "BLECharacteristicSpec.h"
#include "logger.h"
#include "coders.h"

static auto* LOG_TAG = "XboxVibrationsCommand";

BLEEncodeResult encodeVibrationsCommand(const XboxVibrationsCommand& c, size_t& usedBytes, uint8_t buffer[], size_t bufferLen);

const BLEValueEncoder<XboxVibrationsCommand> XboxVibrationsCommand::Encoder(encodeVibrationsCommand);
const BLECharacteristicSpec XboxVibrationsCommand::CharSpec{
    .serviceUUID = NimBLEUUID(blegc::hidSvcUUID),
    .characteristicUUID = NimBLEUUID(blegc::inputReportChrUUID),
    .properties = BLE_GATT_CHR_PROP_WRITE};

constexpr size_t vibrationsDataLen = 8;

inline uint8_t encodeMotorEnable(float power, int bit) {
  return (power > 0.0f) * 1 << bit;
}

inline uint8_t encodeMotorPower(float power) {
  return static_cast<uint8_t>(max(min(power, 1.0f), 0.0f) * 100.0f);
}

inline uint8_t encodeDuration(uint32_t durationMs) {
  return static_cast<uint8_t>(min(durationMs, static_cast<uint32_t>(2550)) / 10);
}

BLEEncodeResult encodeVibrationsCommand(const XboxVibrationsCommand& c, size_t& usedBytes, uint8_t buffer[], size_t bufferLen) {
  if (bufferLen < vibrationsDataLen) {
    BLEGC_LOGD(LOG_TAG, "Expected buffer of at least %d bytes, was %d bytes", vibrationsDataLen, bufferLen);
    return BLEEncodeResult::BufferTooShort;
  }

  if (c.cycles == 0) {
    buffer[0] = 0;
  } else {
    buffer[0] = encodeMotorEnable(c.rightMotor, 0) | encodeMotorEnable(c.leftMotor, 1) |
                   encodeMotorEnable(c.rightTriggerMotor, 2) | encodeMotorEnable(c.leftTriggerMotor, 3);
  }

  buffer[1] = encodeMotorPower(c.leftTriggerMotor);
  buffer[2] = encodeMotorPower(c.rightTriggerMotor);
  buffer[3] = encodeMotorPower(c.leftMotor);
  buffer[4] = encodeMotorPower(c.rightMotor);
  buffer[5] = encodeDuration(c.durationMs);
  buffer[6] = encodeDuration(c.pauseMs);
  buffer[7] = c.cycles == 0 ? 0 : c.cycles - 1;

  usedBytes = 8;

  return BLEEncodeResult::Success;
}
