#pragma once

enum class BLEAutoScanNotification : uint8_t {
  Auto = 1,
  Enabled = 2,
  Disabled = 3,
  ScanStopped = 4,
  ScanFinished = 5,
};

enum class BLEUserCallbackKind : uint8_t {
  ControllerConnecting = 1,
  ControllerConnectionFailed = 2,
  ControllerConnected = 3,
  ControllerDisconnected = 4,

  ScanStarted = 5,
  ScanStopped = 6,
};

struct BLEUserCallback {
  BLEUserCallbackKind kind;
  BLEAbstractController* pCtrl;
};
