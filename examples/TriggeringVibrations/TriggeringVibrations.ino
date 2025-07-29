#include <Arduino.h>
#include <BLEControllerRegistry.h>

Controller controller;

void setup(void) {
  Serial.begin(115200);
  controller.begin();
}

int i = 0;

void loop() {
  if (controller.isConnected()) {
    VibrationsCommand cmd;

    switch (i % 4) {
      case 0: cmd.rightMotor = 1.0f; break; // 1.0f = max power for the motor
      case 1: cmd.leftMotor = 1.0f; break;
      case 2: cmd.leftTriggerMotor = 1.0f; break;
      case 3: cmd.rightTriggerMotor = 1.0f; break;
    }

    cmd.durationMs = 500;
    controller.writeVibrations(cmd);
  }
  i++;
  delay(1000);
}
