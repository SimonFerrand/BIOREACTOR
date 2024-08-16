+/*
  Instructions to connect the relay and control a 5V LED grow light (puce 2835) with Arduino:

  1. Connections for the relay to Arduino:
     - Relay VCC: Connect to 5V on the Arduino.
     - Relay GND: Connect to GND on the Arduino.
     - Relay IN (control signal): Connect to digital pin 27 on the Arduino.

  2. Connections for the LED light to the relay:
     - Red Wire (5V) from LED light: Connect to the normally open (NO) terminal of the relay.
     - Black Wire (GND) from LED light: Connect to the GND of the power source.
     - Power source 5V: Connect the positive terminal to the common (COM) terminal of the relay.
     - Power source GND: Connect the negative terminal to the GND of the Arduino and the GND of the LED light.

  This code will turn the LED light on or off based on commands received from the serial monitor.
*/

const int relayPin = 10; // Pin connected to the relay

void setup() {
  Serial.begin(115200);  // Start the serial communication at 115200 baud rate
  pinMode(relayPin, OUTPUT); // Set the relay pin as an output
  digitalWrite(relayPin, HIGH); // Ensure the relay is ON at the start
  Serial.println("LED Grow Light is turned ON permanently.");
}

void loop() {
  // No actions needed in the loop since the LED light is always on
}