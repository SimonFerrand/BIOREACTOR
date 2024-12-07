// SensorController.cpp
#include "SensorController.h"
#include "Logger.h"

DS18B20TemperatureSensor* SensorController::waterTempSensor = nullptr;

void SensorController::initialize(DS18B20TemperatureSensor& waterTemp) {
    waterTempSensor = &waterTemp;
}

void SensorController::beginAll() {
    waterTempSensor->begin();
}

float SensorController::readSensor(const String& sensorName) {
    SensorInterface* sensor = findSensorByName(sensorName);
    if (sensor) {
        float value = sensor->readValue();
        return value;
    }
    Logger::log(Logger::LogLevel::WARNING, "Sensor not found: " + sensorName);
    return 0.0f;
}

void SensorController::updateAllSensors() {
    readSensor(waterTempSensor->getName());
}

SensorInterface* SensorController::findSensorByName(const String& name) {
    if (name == waterTempSensor->getName()) return waterTempSensor;
    return nullptr;
}
