#pragma once

#include <cstdint>
#include <cstddef>

enum class BLEEncodeResult : uint8_t { Success = 0, InvalidValue = 1, BufferTooShort = 2 };

struct BLEBaseCommand {
  virtual ~BLEBaseCommand() = default;

  virtual BLEEncodeResult encode(size_t& usedBytes, uint8_t buffer[], size_t bufferLen) = 0;
};
