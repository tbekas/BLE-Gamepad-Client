#pragma once

#include <stdint.h>
#include <functional>
#include <vector>

template <class T>
using SignalDecoder = std::function<bool(T&, uint8_t payload[], size_t payloadLen)>;

template <class T>
using SignalEncoder = std::function<size_t(const T& value, uint8_t outBuffer[], size_t bufferLen)>;
