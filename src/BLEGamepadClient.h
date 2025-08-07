#pragma once

class BLEGamepadClient {
 public:
  BLEGamepadClient() = delete;

  static void enableAutoScan();
  static void disableAutoScan();
  static bool isAutoScanEnabled();
  static void deleteBonds();
  static bool addControllerAdapter(const BLEControllerAdapter& adapter);

};
