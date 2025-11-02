#include "XboxVibrationsCommand.h"

#include <NimBLEDevice.h>
#include "logger.h"

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

BLEEncodeResult XboxVibrationsCommand::encode(size_t& usedBytes, uint8_t buffer[], size_t bufferLen) {
  if (bufferLen < vibrationsDataLen) {
    BLEGC_LOGD("Expected buffer of at least %d bytes, was %d bytes", vibrationsDataLen, bufferLen);
    return BLEEncodeResult::BufferTooShort;
  }

  if (this->cycles == 0) {
    buffer[0] = 0;
  } else {
    buffer[0] = encodeMotorEnable(this->rightMotor, 0) | encodeMotorEnable(this->leftMotor, 1) |
                encodeMotorEnable(this->rightTriggerMotor, 2) | encodeMotorEnable(this->leftTriggerMotor, 3);
  }

  buffer[1] = encodeMotorPower(this->leftTriggerMotor);
  buffer[2] = encodeMotorPower(this->rightTriggerMotor);
  buffer[3] = encodeMotorPower(this->leftMotor);
  buffer[4] = encodeMotorPower(this->rightMotor);
  buffer[5] = encodeDuration(this->durationMs);
  buffer[6] = encodeDuration(this->pauseMs);
  buffer[7] = this->cycles == 0 ? 0 : this->cycles - 1;

  usedBytes = 8;

  return BLEEncodeResult::Success;
}
