#pragma once

#include <NimBLEAddress.h>
#include <memory>
#include "logger.h"

struct BLEBaseEvent {
  /// @brief Peer address of the controller that send this event.
  NimBLEAddress controllerAddress{};

#if CONFIG_BT_BLEGC_COPY_REPORT_DATA > 0
  std::shared_ptr<uint8_t[]> data{nullptr};
  size_t dataLen{0};
  size_t dataCap{0};
#endif

  /// @brief Prints the report data attached to this event. To use this function set the config param
  /// CONFIG_BT_BLEGC_COPY_REPORT_DATA to 1.
  void printReportHexdump() const {
#if CONFIG_BT_BLEGC_COPY_REPORT_DATA > 0
    char buf[] = "00000000";
    for (size_t j = 0; j < dataLen; ++j) {
      CONFIG_BT_BLEGC_LOGGER("%02d: ", j % 100);
      uint8_t value = data.get()[j];
      for (int i = 7; i >= 0; --i) {
        buf[7 - i] = value >> i & 1 ? '1' : '0';
      }
      CONFIG_BT_BLEGC_LOGGER(buf);
      CONFIG_BT_BLEGC_LOGGER(" 0x%02x\n", value);
    }
#else
    CONFIG_BT_BLEGC_LOGGER("To use printReportHexdump set CONFIG_BT_BLEGC_COPY_REPORT_DATA to 1\n");
#endif
  }
};
