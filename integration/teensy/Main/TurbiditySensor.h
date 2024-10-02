/*
 * TurbiditySensor.h
 * This file defines a class for reading turbidity values from a turbidity sensor.
 * The class implements the SensorInterface and provides methods to initialize the sensor and read the turbidity value.
 *
 * Physical modules used:
 * - Gravity: Analog Turbidity Sensor
 */

/*
Installation Instructions:

For the Gravity: Analog Turbidity Sensor
@https://wiki.dfrobot.com/Turbidity_sensor_SKU__SEN0189
1. Connect the sensor output to an analog pin on the Arduino (A2 in this case).
2. Connect V+ to 5V on the Arduino and GND to GND.
*/

#ifndef TURBIDITYSENSOR_H
#define TURBIDITYSENSOR_H

#include "SensorInterface.h"
#include <Arduino.h>

class TurbiditySensor : public SensorInterface {
public:
    /*
     * Constructor for TurbiditySensor.
     * @param pin: The analog pin connected to the turbidity sensor.
     */
    TurbiditySensor(int pin, const char* name);

    /*
     * Method to initialize the turbidity sensor.
     */
    void begin() override;

    /*
     * Method to read the turbidity value from the sensor.
     * @return: The turbidity voltage.
     */
    float readValue();

    const char* getName() const override { return _name; }

private:
    int _pin; // Analog pin connected to the turbidity sensor
    const char* _name;
};

#endif


/*
Installation Instructions:

For the Gravity: Analog Turbidity Sensor
@https://wiki.dfrobot.com/Turbidity_sensor_SKU__SEN0189
1. Connect the sensor output to an analog pin on the Arduino (A2 in this case).
2. Connect V+ to 5V on the Arduino and GND to GND.
*/

