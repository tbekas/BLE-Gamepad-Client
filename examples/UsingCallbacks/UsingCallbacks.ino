#include <Arduino.h>
#include <BLEGamepadClient.h>

void onConnecting(XboxController& ctrl) {
  Serial.printf("controller connecting, address: %s\n", ctrl.getAddress().toString().c_str());
}

void onConnectionFailed(XboxController& ctrl) {
  Serial.printf("controller connection failed, address: %s\n", ctrl.getLastAddress().toString().c_str());
}

void onConnected(XboxController& ctrl) {
  Serial.printf("controller connected, address: %s\n", ctrl.getAddress().toString().c_str());
}

void onDisconnected(XboxController& ctrl) {
  Serial.printf("controller disconnected, address: %s\n", ctrl.getLastAddress().toString().c_str());
}

void onUpdate(XboxControlsEvent& e) {
  Serial.printf("lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
    e.leftStickX, e.leftStickY, e.rightStickX, e.rightStickY);
}

void onScanStarted() {
  Serial.printf("scan started\n");
}

void onScanStopped() {
  Serial.printf("scan stopped\n");
}

XboxController controller;

void setup(void) {
  Serial.begin(115200);
  controller.begin();
  controller.onConnecting(onConnecting);              // connecting started
  controller.onConnectionFailed(onConnectionFailed);  // connection failed
  controller.onConnected(onConnected);                // connection succeded
  controller.onDisconnected(onDisconnected);          // disconnected
  controller.onUpdate(onUpdate);                      // value updated

  auto* pAutoScan = BLEGamepadClient::getAutoScan();
  pAutoScan->onScanStarted(onScanStarted);
  pAutoScan->onScanStopped(onScanStopped);
}

void loop() {
  delay(1000);
}
