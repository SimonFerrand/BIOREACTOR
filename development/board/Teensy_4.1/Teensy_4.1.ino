// Test program to blink the internal LED on the Teensy 4.1
// 
// Instructions to install the Teensy 4.1 in Arduino IDE:
// 1. Open Arduino IDE (version 2.3 or higher).
// 2. Go to "File" > "Preferences".
// 3. In the "Additional Boards Manager URLs" field, add the following URL:
//    https://www.pjrc.com/teensy/package_teensy_index.json
// 4. Click "OK" to save the preferences.
// 5. Go to "Tools" > "Board" > "Board Manager".
// 6. In the Board Manager, search for "Teensy" and install the Teensy platform.
// 7. Select "Teensy 4.1" under "Tools" > "Board".
// 8. Upload the code to your Teensy 4.1 to test the internal LED.

void setup() {
  pinMode(13, OUTPUT); // Set pin 13 as an output (internal LED)
}

void loop() {
  digitalWrite(13, HIGH);  // Turn on the LED
  delay(2000);             // Wait for 2 seconds
  digitalWrite(13, LOW);   // Turn off the LED
  delay(500);              // Wait for 500 milliseconds
}