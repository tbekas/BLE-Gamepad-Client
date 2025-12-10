#pragma once

#include <cstdint>

enum class BLEEncodeResult : uint8_t { Success = 0, InvalidValue = 1, BufferTooShort = 2 };

struct BLEBaseCommand {
  // TODO add field for encoded data
};
