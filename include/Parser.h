#pragma once

#include <vector>
#include <functional>
#include <stdint.h>

template<class T>
using Parser = std::function<T(uint8_t data[], size_t len)>;
