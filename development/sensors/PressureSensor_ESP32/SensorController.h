// SensorController.h
#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

#include <Arduino.h>
#include "PressureSensor.h"

class SensorController {
public:
    /**
     * Initialize the controller with a pressure sensor
     * @param pressureSensor Reference to the pressure sensor instance
     */
    static void initialize(PressureSensor& pressureSensor);
    
    /**
     * Read value from a sensor by name
     * @param sensorName Name of the sensor to read
     * @return Sensor value or 0.0 if sensor not found
     */
    static float readSensor(const String& sensorName);
    
    /**
     * Update readings from all sensors
     */
    static void updateAllSensors();
    
    /**
     * Initialize all sensors
     */
    static void beginAll();
    
    /**
     * Find a sensor by its name
     * @param name Name of the sensor to find
     * @return Pointer to the sensor or nullptr if not found
     */
    static SensorInterface* findSensorByName(const String& name);

private:
    static PressureSensor* pressureSensor;  // Pointer to the pressure sensor instance
};

#endif // SENSOR_CONTROLLER_H