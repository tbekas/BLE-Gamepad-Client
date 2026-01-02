#include <Arduino.h>
#include <BLEGamepadClient.h>

XboxController controller;

void onValueChanged(XboxControlsState &s) {
  Serial.printf("lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
    s.leftStickX, s.leftStickY, s.rightStickX, s.rightStickY);
}

void setup(void) {
  Serial.begin(115200);
  controller.begin();
  controller.onValueChanged(onValueChanged);
}

void loop() {
  delay(100);
}
