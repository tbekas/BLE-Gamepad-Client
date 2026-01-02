#include <Arduino.h>
#include <BLEGamepadClient.h>

XboxController controller;

void setup(void) {
  Serial.begin(115200);
  controller.begin();
}

int i = 0;

void loop() {
  if (controller.isConnected()) {
    XboxVibrationsCommand cmd;

    switch (i % 4) {
      case 0: cmd.rightMotor = 1.0f; break; // 1.0f = 100% power on the motor
      case 1: cmd.leftMotor = 1.0f; break;
      case 2: cmd.leftTriggerMotor = 1.0f; break;
      case 3: cmd.rightTriggerMotor = 1.0f; break;
    }

    cmd.durationMs = 500;
    controller.write(cmd);

    Serial.printf("rm: %.2f, lm: %.2f, ltm: %.2f, rtm: %.2f\n",
      cmd.rightMotor, cmd.leftMotor, cmd.leftTriggerMotor, cmd.rightTriggerMotor);
  } else {
    Serial.println("controller not connected");
  }

  i++;
  delay(1000);
}
