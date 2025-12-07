#pragma once

enum class BLEAutoScanNotification : uint8_t {
  Auto = 0,
  Enabled = 1,
  Disabled = 2,
  ScanStopped = 3,
  ScanFinished = 4
};

enum class BLEUserCallbackKind : uint8_t {
  ControllerConnecting = 0,
  ControllerConnectionFailed = 1,
  ControllerConnected = 2,
  ControllerDisconnected = 3,

  ScanStarted = 4,
  ScanStopped = 5,
};

struct BLEUserCallback {
  BLEUserCallbackKind kind;
  BLEAbstractController* pCtrl;
};
