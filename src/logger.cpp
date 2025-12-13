#include "logger.h"

#include <esp_log.h>
#include <cstdint>

namespace blegc {

void setDefaultLogLevel() {
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
void setLogLevelDebug() {
  esp_log_level_set(BLEGC_LOG_TAG, ESP_LOG_DEBUG);
}

void logBufferHex(const esp_log_level_t level, const char* tag, const uint8_t* buf, std::size_t bufLen) {
  static const char hex[] = "0123456789abcdef";
  constexpr std::size_t BYTES_PER_LINE = 16;

  char line[BYTES_PER_LINE * 3 + 1];  // "xx xx ... " + '\0'

  while (bufLen > 0) {
    std::size_t lineLen = bufLen < BYTES_PER_LINE ? bufLen : BYTES_PER_LINE;
    char* p = line;

    for (std::size_t i = 0; i < lineLen; ++i) {
      uint8_t v = buf[i];
      *p++ = hex[v >> 4];
      *p++ = hex[v & 0x0f];
      *p++ = ' ';
    }

    *p = '\0';

    ESP_LOG_LEVEL(level, tag, "%s", line);

    buf += lineLen;
    bufLen -= lineLen;
  }
}

void logBufferBin(const esp_log_level_t level, const char* tag, const uint8_t* buf, const std::size_t bufLen) {
  char line[9];  // 8 bits + '\0'

  for (std::size_t i = 0; i < bufLen; ++i) {
    const uint8_t v = buf[i];

    line[0] = (v & 0x80) ? '1' : '0';
    line[1] = (v & 0x40) ? '1' : '0';
    line[2] = (v & 0x20) ? '1' : '0';
    line[3] = (v & 0x10) ? '1' : '0';
    line[4] = (v & 0x08) ? '1' : '0';
    line[5] = (v & 0x04) ? '1' : '0';
    line[6] = (v & 0x02) ? '1' : '0';
    line[7] = (v & 0x01) ? '1' : '0';
    line[8] = '\0';

    ESP_LOG_LEVEL(level, tag, "%s", line);
  }
}

}  // namespace blegc
