#include <Arduino.h>
#include <BLEGamepadClient.h>

SteamController controller;

void setup(void) {
  Serial.begin(115200);
  controller.begin();
}

void loop() {
  if (controller.isConnected()) {
    SteamControlsState s;
    controller.read(&s);

    Serial.printf("stick: %.2f,%.2f, lpad: %.2f,%.2f, rpad: %.2f,%.2f\n",
      s.stickX, s.stickY, s.leftPadX, s.leftPadY, s.rightPadX, s.rightPadY);
  } else {
    Serial.println("controller not connected");
  }
  delay(100);
}
