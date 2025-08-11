#include <Arduino.h>
#include <BLEGamepadClient.h>

void onControlsUpdate(BLEControlsEvent& e) {
  Serial.printf("lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
    e.leftStickX, e.leftStickY, e.rightStickX, e.rightStickY);
}

void onConnect(NimBLEAddress address) {
  Serial.printf("controller connected, address: %s\n", address.toString().c_str());
}

void onDisconnect(NimBLEAddress address) {
  Serial.printf("controller disconnected, address: %s\n", address.toString().c_str());
}

BLEController controller;

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
