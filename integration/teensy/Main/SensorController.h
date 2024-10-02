#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

#include <Arduino.h>
#include "PT100Sensor.h"
#include "DS18B20TemperatureSensor.h"
#include "PHSensor.h"
#include "OxygenSensor.h"
#include "AirFlowSensor.h"
#include "TurbiditySensorSEN0554.h"

class SensorController {
public:
    static void initialize(PT100Sensor& waterTemp, DS18B20TemperatureSensor& airTemp, DS18B20TemperatureSensor& electronicTempSensor,
                           PHSensor& ph,
                           OxygenSensor& oxygen, 
                           AirFlowSensor& airFlow, 
                           TurbiditySensorSEN0554& turbiditySEN0554);
    
    static float readSensor(const String& sensorName);
    static void updateAllSensors();
    static void beginAll();
    
    static SensorInterface* findSensorByName(const String& name);

    static void takeSample();

private:
    static PT100Sensor* waterTempSensor;
    static DS18B20TemperatureSensor* airTempSensor;
    static DS18B20TemperatureSensor* electronicTempSensor;
    static PHSensor* phSensor;
    static OxygenSensor* oxygenSensor;
    static AirFlowSensor* airFlowSensor;
    static TurbiditySensorSEN0554* turbiditySensorSEN0554;

    static const unsigned long PUMP_RUNTIME = 3000; // 5 seconds to prime the pump
    static const unsigned long STABILIZATION_TIME = 1500; // 1 seconds to stabilise the sample


};

#endif // SENSOR_CONTROLLER_H