#include "SensorController.h"

// Static pointers, initialized to nullptr
PHSensor* SensorController::phSensor = nullptr;
OxygenSensor* SensorController::oxygenSensor = nullptr;


// Initialize method
void SensorController::initialize(PHSensor& ph,
                                  OxygenSensor& oxygen 
                                  ) {
    // Assign addresses of sensor objects to the static pointers
    phSensor = &ph;
    oxygenSensor = &oxygen;
}

void SensorController::beginAll() {
    phSensor->begin();
    oxygenSensor->begin();
}

float SensorController::readSensor(const String& sensorName, float temperature) {
    SensorInterface* sensor = findSensorByName(sensorName);
    if (sensor) {
        float value = sensor->readValue(temperature);
        return value;
    }
    Serial.println("Sensor not found: " + sensorName);
    return 0.0f;
}

/*
void SensorController::updateAllSensors() {
    readSensor(phSensor->getName()), ;
    readSensor(oxygenSensor->getName());
}
*/

SensorInterface* SensorController::findSensorByName(const String& name) {
    if (name == phSensor->getName()) return phSensor;
    if (name == oxygenSensor->getName()) return oxygenSensor;
    return nullptr;
}


