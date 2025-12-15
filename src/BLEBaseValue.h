#pragma once

#include <NimBLEAddress.h>
#include <memory>

enum class BLEEncodeResult : uint8_t { Success = 0, InvalidValue = 1, BufferTooShort = 2 };

enum class BLEDecodeResult : uint8_t { Success = 0, InvalidReport = 1, NotSupported = 2 };

struct BLEBaseValue {
  virtual ~BLEBaseValue() = default;
  /// @brief Peer address of the controller that send this value or that this value is send to.
  NimBLEAddress controllerAddress{};

#if CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED
  std::shared_ptr<uint8_t[]> reportData{nullptr};
  size_t reportDataLen{0};
  size_t reportDataCap{0};
#endif

  /// @brief Logs in a hexadecimal format the report data attached to this value. To use this function set the config
  /// param CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED to 1.
  void logReportDataHex() const;

  /// @brief Logs in a binary format the report data attached to this value. To use this function set the config
  /// param CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED to 1.
  void logReportDataBin() const;
};
