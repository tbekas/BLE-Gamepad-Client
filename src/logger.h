#pragma once

#include "config.h"

#if CONFIG_BT_BLEGC_LOG_LEVEL >= 5
#define BLEGC_LOGT(tag, format, ...) CONFIG_BT_BLEGC_LOGGER("T %s: " format "\n", tag, ##__VA_ARGS__)
#else
#define BLEGC_LOGT(tag, format, ...) (void)0
#endif

#if CONFIG_BT_BLEGC_LOG_LEVEL >= 4
#define BLEGC_LOGD(tag, format, ...) CONFIG_BT_BLEGC_LOGGER("D %s: " format "\n", tag, ##__VA_ARGS__)
#else
#define BLEGC_LOGD(tag, format, ...) (void)0
#endif

#if CONFIG_BT_BLEGC_LOG_LEVEL >= 3
#define BLEGC_LOGI(tag, format, ...) CONFIG_BT_BLEGC_LOGGER("I %s: " format "\n", tag, ##__VA_ARGS__)
#else
#define BLEGC_LOGI(tag, format, ...) (void)0
#endif

#if CONFIG_BT_BLEGC_LOG_LEVEL >= 2
#define BLEGC_LOGW(tag, format, ...) CONFIG_BT_BLEGC_LOGGER("W %s: " format "\n", tag, ##__VA_ARGS__)
#else
#define BLEGC_LOGW(tag, format, ...) (void)0
#endif

#if CONFIG_BT_BLEGC_LOG_LEVEL >= 1
#define BLEGC_LOGE(tag, format, ...) CONFIG_BT_BLEGC_LOGGER("E %s: " format "\n", tag, ##__VA_ARGS__)
#else
#define BLEGC_LOGE(tag, format, ...) (void)0
#endif
