#pragma once

#include <functional>

enum class BLEDecodeResult : uint8_t { Success = 0, InvalidReport = 1, NotSupported = 2 };

template <typename T>
using BLEValueDecoder = std::function<BLEDecodeResult(T&, uint8_t payload[], size_t payloadLen)>;

enum class BLEEncodeResult : uint8_t { Success = 0, InvalidValue = 1, BufferTooShort = 2 };

template <typename T>
using BLEValueEncoder =
    std::function<BLEEncodeResult(const T& value, size_t& usedBytes, uint8_t buffer[], size_t bufferLen)>;
