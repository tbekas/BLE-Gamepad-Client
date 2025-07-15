#pragma once

#include <stdint.h>
#include <functional>
#include <vector>

template <class T>
using Parser = std::function<T(uint8_t data[], size_t len)>;
