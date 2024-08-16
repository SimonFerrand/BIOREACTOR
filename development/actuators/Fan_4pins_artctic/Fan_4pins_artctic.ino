#include <Arduino.h>

const int pwmPin = 9;  // PWM control pin connected to the "I" wire of the fan
int targetRPM = 0;     // Variable to store the target RPM entered via Serial Monitor

void setup() {
  pinMode(pwmPin, OUTPUT);   // Set the PWM pin as an output
  Serial.begin(115200);      // Start serial communication at 115200 baud rate
  Serial.println("Enter the desired RPM (200-3300):"); // Prompt for input
}

void loop() {
  // Check if there's any input available in the Serial Monitor
  if (Serial.available() > 0) {
    targetRPM = Serial.parseInt();  // Read the RPM value from the Serial Monitor

    // Clean up any remaining characters in the serial buffer
    while (Serial.available() > 0) {
      Serial.read();  // Flush the buffer to avoid reading residual characters
    }

    targetRPM = constrain(targetRPM, 200, 3300);  // Constrain the RPM value to be within the fan's limits

    int pwmValue = rpmToPWM(targetRPM);  // Convert the RPM value to a corresponding PWM value
    analogWrite(pwmPin, pwmValue);  // Apply the PWM value to the fan

    // Print the target RPM and the corresponding PWM value to the serial monitor
    Serial.print("RPM set to: ");
    Serial.print(targetRPM);
    Serial.print(" corresponds to PWM value: ");
    Serial.println(pwmValue);
  }
}

// Function to convert target RPM to PWM value
int rpmToPWM(int targetRPM) {
  // Map the RPM range (200-3300) to the PWM range (0-255)
  int pwmValue = map(targetRPM, 200, 3300, 0, 255);
  return pwmValue;
}
