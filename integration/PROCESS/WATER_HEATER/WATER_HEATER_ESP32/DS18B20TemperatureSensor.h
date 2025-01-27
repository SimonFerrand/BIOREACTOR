/*
 * DS18B20TemperatureSensor.h
 * This file defines a class for reading temperature from the DS18B20 sensor.
 * The class implements the SensorInterface and provides methods to initialize the sensor and read the temperature.
 *
 * Physical modules used:
 * - DS18B20 Temperature Sensor
 */

/*
Installation Instructions:

For the DS18B20 Temperature Sensor:
@https://wiki.dfrobot.com/Waterproof_DS18B20_Digital_Temperature_Sensor__SKU_DFR0198_
1. Connect the DS18B20 to the Arduino using a plugable terminal sensor adapter or directly by wiring.
    - Red Wire (VCC) to 5V or 3.3V on the Arduino.
    - Black Wire (GND) to GND on the Arduino.
    - Blue Wire (Data Signal) to a digital pin on the Arduino (Pin 52 in this case).
2. Use "Gravity: Terminal Sensor Adapter V2.0" @https://www.dfrobot.com/product-203.html
        or Place a 4.7kΩ pull-up resistor between the VCC and Data Signal lines to ensure proper communication.
*/

#ifndef DS18B20TEMPERATURESENSOR_H
#define DS18B20TEMPERATURESENSOR_H

#include "SensorInterface.h"
#include <OneWire.h>
#include <Arduino.h>

class DS18B20TemperatureSensor : public SensorInterface {
public:
    /*
     * Constructor for DS18B20TemperatureSensor.
     * @param pin: The digital pin connected to the DS18B20 sensor.
     */
    DS18B20TemperatureSensor(int pin, const char* name);
    /*
     * Method to initialize the temperature sensor.
     */
    void begin();

    /*
     * Method to read the temperature from the sensor.
     * @return: The temperature in degrees Celsius.
     */
    float readValue();
    const char* getName() const override { return _name; }

private:
    OneWire _ds; // OneWire object for communication with DS18B20
    int _pin;    // Digital pin connected to the DS18B20
    const char* _name;

    float _lastValidTemp = 20.0f;  // Stocke la dernière température valide mesurée, initialisée à 20°C
    static constexpr float MIN_VALID_TEMP = -10.0f;  // Température minimum acceptable 
    static constexpr float MAX_VALID_TEMP = 120.0f;  // Température maximum acceptable
    static constexpr float MAX_TEMP_DELTA = 5.0f;    // Variation maximum autorisée entre 2 mesures
};

#endif

/*
 *Installation Instructions:
 *
 *For the DS18B20 Temperature Sensor:
 *@https://wiki.dfrobot.com/Waterproof_DS18B20_Digital_Temperature_Sensor__SKU_DFR0198_
 *1. Connect the DS18B20 to the Arduino using a plugable terminal sensor adapter or directly by wiring.
    - Red Wire (VCC) to 5V or 3.3V on the Arduino.
    - Black Wire (GND) to GND on the Arduino.
    - Blue Wire (Data Signal) to a digital pin on the Arduino (Pin 52 in this case).
  2. Use "Gravity: Terminal Sensor Adapter V2.0" @https://www.dfrobot.com/product-203.html
      or Place a 4.7kΩ pull-up resistor between the VCC and Data Signal lines to ensure proper communication.
 */



