/*
  https://cdn-learn.adafruit.com/downloads/pdf/adafruit-max31865-rtd-pt100-amplifier.pdf
  https://learn.adafruit.com/adafruit-max31865-rtd-pt100-amplifier/pinouts
  https://learn.adafruit.com/adafruit-max31865-rtd-pt100-amplifier/

  Instructions to prepare the MAX31865 module for a 3-wire PT100 sensor:
  
  1. Solder the "2/3 Wire" jumper:
     - On the MAX31865 module PCB, locate the jumper labeled "2/3 Wire".
     - Use a soldering iron to deposit a small amount of solder on the two pads of the "2/3 Wire" jumper to connect them together.

  2. Cut the wire connecting the left side of the 2-way jumper above Rref:
     - Find the 2-way jumper just above the reference resistor (Rref).
     - Use a track cutter or a sharp tool to carefully cut the track on the PCB that connects the left side of the 2-way jumper.
     - Make sure the cut is clean and the track is completely interrupted.

  3. Solder the right side of the 2-way jumper labeled "3":
     - Deposit a small amount of solder to connect the pads on the right side of the 2-way jumper, labeled "3".

  Hardware connections:
  
  1. Connections from the MAX31865 module to the Arduino Mega (software SPI):
     - CS (Chip Select): Connect to pin 22 on the Arduino Uno.
     - DI (Data Input, equivalent to MOSI - Master Out Slave In): Connect to pin 23 on the Arduino Uno.
     - DO (Data Output, equivalent to MISO - Master In Slave Out): Connect to pin 24 on the Arduino Uno.
     - CLK (Clock): Connect to pin 25 on the Arduino Uno.
     - Vin: Connect to 5V on the Arduino Uno.
     - GND: Connect to GND on the Arduino Uno.

  2. Connections from the 3-wire PT100 sensor to the MAX31865 module:
     - Use a multimeter to measure the resistance between the wires of the PT100 sensor.
     - Find the two wires that have very low resistance between them (about 2 ohms). These two wires are directly connected together.
     - Connect these two wires to the F+ and RTD+ terminals on the MAX31865 module (order does not matter).
     - Connect the third wire (which has higher resistance when measured with each of the other two wires) to the F- or RTD- terminal on the MAX31865 module.
*/

// Include necessary libraries
#include <SPI.h>
#include <Adafruit_MAX31865.h>

// Use software SPI: CS, DI, DO, CLK
//Adafruit_MAX31865 thermo = Adafruit_MAX31865(22, 23, 24, 25);
Adafruit_MAX31865 thermo = Adafruit_MAX31865(5, 23, 19, 18);


void setup() {
  Serial.begin(115200);  // Start the serial communication at 115200 baud rate
  Serial.println("Initializing MAX31865...");
  thermo.begin(MAX31865_3WIRE);  // Set up the MAX31865 module for 3-wire RTD
  Serial.println("MAX31865 Initialized.");
}

void loop() {
  // Read the temperature
  float temperature = thermo.temperature(100, 430); // Parameters are Rref and RTD nominal resistance
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  // Check for faults
  checkFault();

  delay(1000);  // Wait for one second before reading again
}

void checkFault() {
  uint8_t fault = thermo.readFault();
  if (fault) {
    Serial.print("Fault 0x"); Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold");
    }
    if (fault & MAX31865_FAULT_LOWTHRESH) {
      Serial.println("RTD Low Threshold");
    }
    if (fault & MAX31865_FAULT_REFINLOW) {
      Serial.println("REFIN- > 0.85 x Bias");
    }
    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
    }
    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - RTD disconnected");
    }
    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage");
    }
    thermo.clearFault();
  }
}
