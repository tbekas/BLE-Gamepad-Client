#pragma once
#include <esp_log.h>
#include <cstdint>

#ifndef CONFIG_BT_BLEGC_LOG_LEVEL
#if defined(CORE_DEBUG_LEVEL)
#define CONFIG_BT_BLEGC_LOG_LEVEL CORE_DEBUG_LEVEL
#else
#define CONFIG_BT_BLEGC_LOG_LEVEL 1
#endif
#endif

#ifndef CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED
#define CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED 0
#endif

static auto* BLEGC_LOG_TAG = "blegc";

namespace blegc {

void setDefaultLogLevel();

void setLogLevelDebug();

void logBufferHex(esp_log_level_t level, const char* tag, const uint8_t *buf, std::size_t bufLen);

void logBufferBin(esp_log_level_t level, const char* tag, const uint8_t *buf, std::size_t bufLen);

}  // namespace blegc

#define BLEGC_LOGV(format, ...) ESP_LOG_LEVEL(ESP_LOG_VERBOSE, BLEGC_LOG_TAG, format, ##__VA_ARGS__)
#define BLEGC_LOGD(format, ...) ESP_LOG_LEVEL(ESP_LOG_DEBUG, BLEGC_LOG_TAG, format, ## __VA_ARGS__)
#define BLEGC_LOGI(format, ...) ESP_LOG_LEVEL(ESP_LOG_INFO, BLEGC_LOG_TAG, format, ## __VA_ARGS__)
#define BLEGC_LOGW(format, ...) ESP_LOG_LEVEL(ESP_LOG_WARN, BLEGC_LOG_TAG, format, ## __VA_ARGS__)
#define BLEGC_LOGE(format, ...) ESP_LOG_LEVEL(ESP_LOG_ERROR, BLEGC_LOG_TAG, format, ## __VA_ARGS__)

#if CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED
#define BLEGC_LOGD_BUFFER_HEX(buf, bufLen) blegc::logBufferHex(ESP_LOG_DEBUG, BLEGC_LOG_TAG, buf, bufLen)
#define BLEGC_LOGD_BUFFER_BIN(buf, bufLen) blegc::logBufferBin(ESP_LOG_DEBUG, BLEGC_LOG_TAG, buf, bufLen)
#else
#define BLEGC_LOGD_BUFFER_HEX(buf, bufLen) (void)0
#define BLEGC_LOGD_BUFFER_BIN(buf, bufLen) (void)0
#endif
