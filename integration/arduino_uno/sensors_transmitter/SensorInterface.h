/*
 * SensorInterface.h
 * This file defines an abstract interface for sensors, providing a standard way
 * to read values from various sensors in the project. This interface can be implemented
 * by any sensor class, such as a temperature sensor, humidity sensor, etc.
 */

#ifndef SENSORINTERFACE_H
#define SENSORINTERFACE_H

class SensorInterface {
public:
    /*
     * Pure virtual function to initialize the sensor.
     * This should be called before using the sensor.
     */
    virtual void begin() = 0;

    /*
     * Pure virtual function to read the sensor value.
     * @return: The value read from the sensor.
     */
    virtual float readValue(float temperature) = 0;

    /*
     * Virtual function to get the name of the sensor.
     * @return: Constant character pointer to the name of the sensor.
     */
    virtual const char* getName() const = 0;

    /*
     * Virtual destructor to ensure proper cleanup of derived classes.
     */
    virtual ~SensorInterface() {}
};

#endif