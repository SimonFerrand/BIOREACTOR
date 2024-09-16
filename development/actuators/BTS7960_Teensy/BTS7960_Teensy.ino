/* 
 * Program to control a motor using the BTS7960 motor driver with a Teensy 4.1.
 * 
 * Wiring:
 * - Teensy Pin 1 (3.3V logic) -> RPWM (BTS7960).
 * - Ground all devices: Teensy and BTS7960.
 * - BTS7960 powered by 12V for the motor and 5V for the logic inputs.
 * - BTS7960 accepts both 3.3V and 5V PWM signals (Teensy outputs 3.3V PWM).
 * - VCC of the BTS7960 must be powered with 5V, even if the PWM signal is 3.3V.
 * - To control the motor, at least REN and LEN must be powered (5V).
 * - The motor alternates between 25% and 75% duty cycle every 2 seconds.
 * - Serial Monitor will display current motor speed.
 * 
 * Notes:
 * - REN and LEN can be connected to control pins to enable/disable the motor driver. It may be possible to power them with 3.3V, but it's recommended to use 5V for consistent operation.
 * - If REN and LEN are powered by 5V and the PWM signal is 3.3V, this is fine as the BTS7960 can handle 3.3V PWM signals. 
 * 
 * Teensy Installation:
 * 1. Go to Arduino IDE -> Preferences -> Additional Board URLs, and add: 
 *    https://www.pjrc.com/teensy/package_teensy_index.json
 * 2. Open Board Manager, search for 'Teensy', and install Teensyduino.
 * 3. Select 'Teensy 4.1' in Tools -> Board.
 */

#define MOTOR_PWM_PIN 1  // RPWM connected to Pin 1 of Teensy

int motorSpeed25 = 64;    // 25% of 255 (for PWM)
int motorSpeed75 = 255;   // 100% of 255 (for PWM)
bool is25Percent = true;  // Boolean to track motor speed state

unsigned long previousMillis = 0;
const long interval = 4000;  // 2 seconds

void setup() {
  pinMode(MOTOR_PWM_PIN, OUTPUT); // Set PWM pin as output
  Serial.begin(115200);           // Start serial communication

  Serial.println("Motor Control Initialized");
  Serial.println("Alternating between 25% and 100% speed every 2 seconds...");
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Update the last time action was performed

    if (is25Percent) {
      analogWrite(MOTOR_PWM_PIN, motorSpeed25);  // Set motor speed to 25%
      Serial.println("Motor running at 25% speed");
    } else {
      analogWrite(MOTOR_PWM_PIN, motorSpeed75);  // Set motor speed to 100%
      Serial.println("Motor running at 100% speed");
    }

    is25Percent = !is25Percent;  // Toggle between 25% and 100%
  }
}
