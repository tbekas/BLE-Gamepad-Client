#pragma once

#include <vector>
#include <functional>
#include <stdint.h>

// TODO use std::byte instead of uint8_t
template<class T>
using Parser = std::function<T(uint8_t data[], size_t len)>;
