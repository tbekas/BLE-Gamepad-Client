#include "BLEBaseValue.h"

#include "logger.h"

void BLEBaseValue::logReportDataHex() const {
#if CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED
  BLEGC_LOGD_BUFFER_HEX(reportData.get(), reportDataLen);
#else
  BLEGC_LOGW("To use this function set CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED to 1");
#endif
}

void BLEBaseValue::logReportDataBin() const {
#if CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED
  BLEGC_LOGD_BUFFER_BIN(reportData.get(), reportDataLen);
#else
  BLEGC_LOGW("To use this function set CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED to 1");
#endif
}
