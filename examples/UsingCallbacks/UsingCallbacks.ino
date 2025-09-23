#include <Arduino.h>
#include <BLEGamepadClient.h>

void onControlsUpdate(XboxControlsEvent& e) {
  Serial.printf("lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
    e.leftStickX, e.leftStickY, e.rightStickX, e.rightStickY);
}

void onConnect(XboxController& ctrl) {
  Serial.printf("controller connected, address: %s\n", ctrl.getAddress().toString().c_str());
}

void onDisconnect(XboxController& ctrl) {
  Serial.printf("controller disconnected, address: %s\n", ctrl.getLastAddress().toString().c_str());
}

XboxController controller;

void setup(void) {
  Serial.begin(115200);
  controller.begin();
  controller.onConnect(onConnect);
  controller.onDisconnect(onDisconnect);
  controller.onControlsUpdate(onControlsUpdate);
}

void loop() {
  delay(1000);
}
