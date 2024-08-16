// This Arduino sketch configures PWM output to control a DC motor pump using a PW1VA01 module,
// and controls a relay to power the motor setup. The PWM module converts PWM signals to a 0-5V output
// and is powered by 24V. It connects to the Arduino via pin 5.
// The relay controlling the power supply to the motor speed controller is connected to pin 6.

// Function to control the pump with on/off and speed settings
void controlPump(bool pumpOn, int speedPercentage) {
  if (pumpOn && speedPercentage >= 10 && speedPercentage <= 100) {
    // Map the speed percentage from 10-100% to PWM values 26-255
    int pwmValue = map(speedPercentage, 10, 100, 26, 255);
    analogWrite(3, pwmValue);  // Set the speed
    digitalWrite(4, HIGH);     // Turn on the relay to power the pump
    Serial.print("Pump is ON, Speed set to: ");
    Serial.println(speedPercentage);
  } else {
    analogWrite(3, 0);         // Set speed to 0
    digitalWrite(4, LOW);      // Turn off the relay to power down the pump
    Serial.println("Pump is OFF");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(3, OUTPUT); // PWM output for speed control
  pinMode(4, OUTPUT); // Relay control
}

void loop() {
  // Example usage of the controlPump function
  controlPump(true, 50);  // Turn the pump on at 50% speed
  /*delay(5000);            // Run for 5 seconds
  controlPump(false, 0);  // Turn the pump off
  delay(2000);            // Off for 2 seconds
  controlPump(true, 80);  // Turn the pump on at 80% speed
  delay(5000);            // Run for 5 seconds
  controlPump(false, 0);  // Turn the pump off
  delay(2000);            // Off for 2 seconds*/
}
