#pragma once

#include <NimBLEAddress.h>
#include <memory>
#include "logger.h"
#include "utils.h"

enum class BLEDecodeResult : uint8_t { Success = 0, InvalidReport = 1, NotSupported = 2 };

struct BLEBaseEvent {
  virtual ~BLEBaseEvent() = default;
  /// @brief Peer address of the controller that send this event.
  NimBLEAddress controllerAddress{};

  virtual BLEDecodeResult decode(uint8_t data[], size_t dataLen) = 0;

#if CONFIG_BT_BLEGC_ENABLE_DEBUG_DATA
  std::shared_ptr<uint8_t[]> data{nullptr};
  size_t dataLen{0};
  size_t dataCap{0};
#endif

  /// @brief Prints the report data attached to this event. To use this function set the config param
  /// CONFIG_BT_BLEGC_ENABLE_DEBUG_DATA to 1.
  void printReportHexdump() const {
#if CONFIG_BT_BLEGC_ENABLE_DEBUG_DATA
    blegc::printHexdump(data.get(), dataLen);
#else
    CONFIG_BT_BLEGC_LOGGER("To use printReportHexdump set CONFIG_BT_BLEGC_ENABLE_DEBUG_DATA to 1\n");
#endif
  }
};
