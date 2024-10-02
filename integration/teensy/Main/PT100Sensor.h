/*
 * PT100Sensor.h
 * This file defines a class for reading temperature from a PT100 sensor using the MAX31865 module.
 * The class implements the SensorInterface and provides methods to initialize the sensor and read the temperature.
 *
 * Physical modules used:
 * - PT100 Temperature Sensor
 * - MAX31865 Amplifier Module
 */

/*
  Installation Instructions:

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

#ifndef PT100SENSOR_H
#define PT100SENSOR_H

#include "SensorInterface.h"
#include <Adafruit_MAX31865.h>

class PT100Sensor : public SensorInterface {
public:
    /*
     * Constructor for PT100Sensor.
     * @param csPin: Chip Select pin for SPI communication.
     * @param diPin: Data Input (MOSI) pin for SPI communication.
     * @param doPin: Data Output (MISO) pin for SPI communication.
     * @param clkPin: Clock pin for SPI communication.
     */
    PT100Sensor(int csPin, int diPin, int doPin, int clkPin, const char* name);
    const char* getName() const override { return _name; }

    /*
     * Method to initialize the PT100 sensor.
     */
    void begin();

    /*
     * Method to read the temperature from the sensor.
     * @return: The temperature in degrees Celsius.
     */
    float readValue();

private:
    Adafruit_MAX31865 _thermo; // MAX31865 sensor object
    const char* _name;
};

#endif
