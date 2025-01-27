// SensorController.h
#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

#include <Arduino.h>
#include "PT100Sensor.h"
#include "PressureSensor.h"
#include "DS18B20TemperatureSensor.h"

class SensorController {
public:
    static bool initialize(DS18B20TemperatureSensor& waterTemp, PressureSensor& pressure);
    static float readSensor(const String& sensorName);
    static void updateAllSensors();
    static bool beginAll();
    static SensorInterface* findSensorByName(const String& name);

private:
    static DS18B20TemperatureSensor* waterTempSensor;
    static PressureSensor* pressureSensor;

};

#endif