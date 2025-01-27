// SensorController.cpp
#include "SensorController.h"
#include "Logger.h"

DS18B20TemperatureSensor* SensorController::waterTempSensor = nullptr;
PressureSensor* SensorController::pressureSensor = nullptr;

bool SensorController::initialize(DS18B20TemperatureSensor& waterTemp, PressureSensor& pressure) {
    waterTempSensor = &waterTemp;
    pressureSensor = &pressure;
    Logger::log(Logger::LogLevel::INFO, F("Sensors initialized successfully"));
    return true;
}

bool SensorController::beginAll() {
    if (!waterTempSensor || !pressureSensor) {
        Logger::log(Logger::LogLevel::ERROR, F("Sensors not initialized"));
        return false;
    }

    waterTempSensor->begin();
    pressureSensor->begin();

    // First attempt at reading
    bool waterTempOk = false;
    bool pressureOk = false;
    
    // Make 3 attempts to read with delay
    for(int i = 0; i < 3; i++) {
        float tempValue = waterTempSensor->readValue();
        waterTempOk = (tempValue > -100 && tempValue < 150);   // Realistic range
        pressureOk = pressureSensor->isHealthy();

        if (waterTempOk && pressureOk) {
            break;   // Exit if readings are good
        }
        
        if (!waterTempOk) {
            Logger::log(Logger::LogLevel::WARNING, 
                String("Temperature sensor reading invalid, attempt ") + String(i+1));
        }
        if (!pressureOk) {
            Logger::log(Logger::LogLevel::WARNING, 
                String("Pressure sensor reading invalid, attempt ") + String(i+1));
        }
        
        delay(1000);  
    }

    // Logger l'état final
    if (!waterTempOk) {
        Logger::log(Logger::LogLevel::ERROR, F("Temperature sensor failed to initialize"));
    }
    if (!pressureOk) {
        Logger::log(Logger::LogLevel::ERROR, F("Pressure sensor failed to initialize"));
    }

    // Return true even if a sensor has failed
    // The system can operate in degraded mode
    Logger::log(Logger::LogLevel::INFO, 
        String("Sensors started. Temp OK: ") + String(waterTempOk) + 
        String(", Pressure OK: ") + String(pressureOk));
    
    return true;  // Enable the system to start up even with faulty sensors
}

float SensorController::readSensor(const String& sensorName) {
    SensorInterface* sensor = findSensorByName(sensorName);
    if (sensor) {
        float value = sensor->readValue();
        return value;
    }
    Logger::log(Logger::LogLevel::WARNING, String("Sensor not found: ") + String(sensorName));
    return 0.0f;
}

void SensorController::updateAllSensors() {
    readSensor(waterTempSensor->getName());
    readSensor(pressureSensor->getName());
}

SensorInterface* SensorController::findSensorByName(const String& name) {
    if (name == waterTempSensor->getName()) return waterTempSensor;
    if (name == pressureSensor->getName()) return pressureSensor;
    return nullptr;
}
