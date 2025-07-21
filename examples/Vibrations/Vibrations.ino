#include <Arduino.h>
#include <BLEGamepadClient.h>

void setup(void) {
  Serial.begin(115200);
  BLEGamepadClient.begin();
}

void loop() {
  // Note the usage of a reference type `Controller&`
  for (Controller& ctrl : BLEGamepadClient.getControllers()) {
    if (ctrl.isConnected()) {
      ControlsEvent e;
      ctrl.controls().read(e);

      VibrationsCommand cmd;
      cmd.leftTriggerMotor = e.leftTrigger;     // power will be proportional to the trigger pressure
      cmd.rightTriggerMotor = e.rightTrigger;   // power will be proportional to the trigger pressure
      cmd.leftMotor = e.dpadLeft ? 1.0 : 0.0;   // either full power or none
      cmd.rightMotor = e.dpadRight ? 1.0 : 0.0; // either full power or none
      cmd.durationMs = 500;

      ctrl.vibrations().write(cmd);
    }
  }
  delay(1000);
}
