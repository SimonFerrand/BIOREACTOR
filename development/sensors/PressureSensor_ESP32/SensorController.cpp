// SensorController.cpp
#include "SensorController.h"

// Initialize static member
PressureSensor* SensorController::pressureSensor = nullptr;

void SensorController::initialize(PressureSensor& sensor) {
    pressureSensor = &sensor;
}

void SensorController::beginAll() {
    if(pressureSensor) {
        pressureSensor->begin();
    }
}

float SensorController::readSensor(const String& sensorName) {
    SensorInterface* sensor = findSensorByName(sensorName);
    if (sensor) {
        return sensor->readValue();
    }
    Serial.println("Warning: Sensor '" + sensorName + "' not found");
    return 0.0f;
}

void SensorController::updateAllSensors() {
    if(pressureSensor) {
        readSensor(pressureSensor->getName());
    }
}

SensorInterface* SensorController::findSensorByName(const String& name) {
    if(pressureSensor && name == pressureSensor->getName()) {
        return pressureSensor;
    }
    return nullptr;
}