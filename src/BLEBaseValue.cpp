#include "BLEBaseValue.h"

#define LOG_LOCAL_LEVEL 6

#include "logger.h"

void BLEBaseValue::logReportDataHex() const {
#if CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED
  BLEGC_LOGD_BUFFER(reportData.get(), reportDataLen);
#else
  BLEGC_LOGW("To use this function set CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED to 1");
#endif
}

void BLEBaseValue::logReportDataBin() const {
#if CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED
  char buf[] = "00000000";
  for (size_t j = 0; j < reportDataLen; ++j) {
    uint8_t value = reportData.get()[j];
    for (int i = 7; i >= 0; --i) {
      buf[7 - i] = value >> i & 1 ? '1' : '0';
    }
    BLEGC_LOGD("%02d %s %02x", j % 100, buf, value);
  }
#else
  BLEGC_LOGW("To use this function set CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED to 1");
#endif
}
