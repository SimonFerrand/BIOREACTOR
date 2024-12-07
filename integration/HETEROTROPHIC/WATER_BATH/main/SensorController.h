// SensorController.h
#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

#include <Arduino.h>
#include "DS18B20TemperatureSensor.h"

class SensorController {
public:
    static void initialize(DS18B20TemperatureSensor& waterTemp);
    static float readSensor(const String& sensorName);
    static void updateAllSensors();
    static void beginAll();
    static SensorInterface* findSensorByName(const String& name);

private:
    static DS18B20TemperatureSensor* waterTempSensor;

};

#endif