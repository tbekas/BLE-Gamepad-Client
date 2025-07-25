#pragma once

#include <Arduino.h>

#ifndef CONFIG_BT_BLEGC_LOG_LEVEL
#if defined(ARDUINO_ARCH_ESP32) && defined(CORE_DEBUG_LEVEL)
#define CONFIG_BT_BLEGC_LOG_LEVEL CORE_DEBUG_LEVEL
#else
#define CONFIG_BT_BLEGC_LOG_LEVEL 0
#endif
#endif

#ifndef CONFIG_BT_BLEGC_LOGGER
#define CONFIG_BT_BLEGC_LOGGER Serial.printf
#endif

#if CONFIG_BT_BLEGC_LOG_LEVEL >= 5
#define BLEGC_LOGT(format, ...) CONFIG_BT_BLEGC_LOGGER("TRACE " format "\n", ##__VA_ARGS__)
#else
#define BLEGC_LOGT(format, ...) (void)0
#endif

#if CONFIG_BT_BLEGC_LOG_LEVEL >= 4
#define BLEGC_LOGD(format, ...) CONFIG_BT_BLEGC_LOGGER("DEBUG " format "\n", ##__VA_ARGS__)
#else
#define BLEGC_LOGD(format, ...) (void)0
#endif

#if CONFIG_BT_BLEGC_LOG_LEVEL >= 3
#define BLEGC_LOGI(format, ...) CONFIG_BT_BLEGC_LOGGER("INFO " format "\n", ##__VA_ARGS__)
#else
#define BLEGC_LOGI(format, ...) (void)0
#endif

#if CONFIG_BT_BLEGC_LOG_LEVEL >= 2
#define BLEGC_LOGW(format, ...) CONFIG_BT_BLEGC_LOGGER("WARN " format "\n", ##__VA_ARGS__)
#else
#define BLEGC_LOGW(format, ...) (void)0
#endif

#if CONFIG_BT_BLEGC_LOG_LEVEL >= 1
#define BLEGC_LOGE(format, ...) CONFIG_BT_BLEGC_LOGGER("ERROR " format "\n", ##__VA_ARGS__)
#else
#define BLEGC_LOGE(format, ...) (void)0
#endif
