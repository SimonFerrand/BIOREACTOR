#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H

#include <Arduino.h>
#include "PHSensor.h"
#include "OxygenSensor.h"

class SensorController {
public:
    static void initialize(PHSensor& ph,
                           OxygenSensor& oxygen 
                           );
    static float readSensor(const String& sensorName, float temperature);
    //static void updateAllSensors();
    static void beginAll();
    
    static SensorInterface* findSensorByName(const String& name);

private:
    static PHSensor* phSensor;
    static OxygenSensor* oxygenSensor;

};

#endif // SENSOR_CONTROLLER_H