#include <Arduino.h>

// FAN : be quiet! Pure Wings 2 PWM 120 mm 1500rpm (4 pins)
  // The tachometer pin could not be used because the voltage was too low in the state to be read by arduino.
const int pwmPin = 9;  // PWM control pin connected to the "I" wire of the fan

void setup() {
  pinMode(pwmPin, OUTPUT);  // Set the PWM pin as an output
  Serial.begin(115200);       // Start serial communication at 9600 baud rate

  int targetRPM = 250;  // Target fan speed in RPM
  int pwmValue = rpmToPWM(targetRPM);  // Calculate corresponding PWM value for the target RPM
  analogWrite(pwmPin, pwmValue);  // Apply the PWM value to the fan

  // Print the target RPM and the corresponding PWM value to the serial monitor
  Serial.print("RPM set to: ");
  Serial.print(targetRPM);
  Serial.print(" corresponds to PWM value: ");
  Serial.println(pwmValue);
}

void loop() {
  // The loop is intentionally left empty as we do not need to update the speed continuously
}

int rpmToPWM(int targetRPM) {
  float loadPercentage;  // Variable to hold the calculated load percentage

  // Calculate the load percentage based on the target RPM using piecewise linear equations
  if (targetRPM <= 200) {
    // First equation for RPMs from 0% to 32% load
    loadPercentage = (targetRPM - 390) / 1.875;
  } else {
    // Second equation for RPMs from 32% to 100% load
    loadPercentage = (targetRPM - 450) / 15.441 + 32;
  }

  // Convert the load percentage to a PWM value (range 0 to 255)
  int pwmValue = map(int(loadPercentage), 0, 100, 0, 255);
  return pwmValue;
}
