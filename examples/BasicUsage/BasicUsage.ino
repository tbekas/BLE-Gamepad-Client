#include <Arduino.h>
#include <BLEGamepadClient.h>

void onUpdate(ControlsEvent& e) {
  Serial.printf("lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n", e.lx, e.ly, e.rx, e.ry);
}

void onConnected(Controller& controller) {
  controller.controls().subscribe(onUpdate);
}

void setup(void) {
  Serial.begin(115200);
  GamepadClient.setConnectedCallback(onConnected);
  GamepadClient.begin();
}

void loop() {
  delay(1000);
}