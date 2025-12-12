#pragma once
#include <esp_log.h>
#include "config.h"

static auto* BLEGC_LOG_TAG = "blegc";

namespace blegc {

static void setDefaultLogLevel() {
#if CONFIG_BT_BLEGC_LOG_LEVEL == 0
  esp_log_level_set(BLEGC_LOG_TAG, ESP_LOG_NONE);
#elif CONFIG_BT_BLEGC_LOG_LEVEL == 1
  esp_log_level_set(BLEGC_LOG_TAG, ESP_LOG_ERROR);
#elif CONFIG_BT_BLEGC_LOG_LEVEL == 2
  esp_log_level_set(BLEGC_LOG_TAG, ESP_LOG_WARN);
#elif CONFIG_BT_BLEGC_LOG_LEVEL == 3
  esp_log_level_set(BLEGC_LOG_TAG, ESP_LOG_INFO);
#elif CONFIG_BT_BLEGC_LOG_LEVEL == 4
  esp_log_level_set(BLEGC_LOG_TAG, ESP_LOG_DEBUG);
#elif CONFIG_BT_BLEGC_LOG_LEVEL == 5
  esp_log_level_set(BLEGC_LOG_TAG, ESP_LOG_VERBOSE);
#elif CONFIG_BT_BLEGC_LOG_LEVEL >= 6
  esp_log_level_set(BLEGC_LOG_TAG, ESP_LOG_MAX);
#endif
}

static void setLogLevelDebug() {
  esp_log_level_set(BLEGC_LOG_TAG, ESP_LOG_DEBUG);
}

}  // namespace blegc

#define BLEGC_LOGV(format, ...) ESP_LOG_LEVEL(ESP_LOG_VERBOSE, BLEGC_LOG_TAG, format, ##__VA_ARGS__)
#define BLEGC_LOGD(format, ...) ESP_LOG_LEVEL(ESP_LOG_DEBUG, BLEGC_LOG_TAG, format, ## __VA_ARGS__)
#define BLEGC_LOGI(format, ...) ESP_LOG_LEVEL(ESP_LOG_INFO, BLEGC_LOG_TAG, format, ## __VA_ARGS__)
#define BLEGC_LOGW(format, ...) ESP_LOG_LEVEL(ESP_LOG_WARN, BLEGC_LOG_TAG, format, ## __VA_ARGS__)
#define BLEGC_LOGE(format, ...) ESP_LOG_LEVEL(ESP_LOG_ERROR, BLEGC_LOG_TAG, format, ## __VA_ARGS__)
#define BLEGC_LOGD_BUFFER(buf, bufLen) ESP_LOG_BUFFER_HEX_LEVEL(BLEGC_LOG_TAG, buf, bufLen, ESP_LOG_DEBUG)
