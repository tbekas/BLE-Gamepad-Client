#pragma once

enum class BLEAutoScanNotification : uint8_t { Auto = 0, ScanFinished = 1 };

enum class BLEUserCallbackKind : uint8_t {
  BLEControllerConnecting = 0,
  BLEControllerConnectionFailed = 1,
  BLEControllerConnected = 2,
  BLEControllerDisconnected = 3,

  BLEScanStarted = 4,
  BLEScanStopped = 5,
};

struct BLEUserCallback {
  BLEUserCallbackKind kind;
  BLEAbstractController* pCtrl;
};
