// SafetySystem.cpp
#include "SafetySystem.h"

SafetySystem::SafetySystem(StateMachine& stateMachine)
    : lastCheckTime(0)
    , checkInterval(1000)
    , alarmEnabled(true)
    , warningEnabled(true)
    , _stateMachine(stateMachine)
{
}

void SafetySystem::safetyTask(void* parameter) {
    SafetySystem* safety = static_cast<SafetySystem*>(parameter);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (true) {
        safety->checkLimits();
        vTaskDelayUntil(&xLastWakeTime, safety->taskFrequency);
    }
}

bool SafetySystem::begin() {
    return TaskManager::createTask(
        safetyTask,
        "SafetyCheck",
        STACK_SIZE_MONITOR,
        this,
        TASK_PRIORITY_HIGH,
        SENSOR_CORE
    );
}

void SafetySystem::checkLimits() {
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime < checkInterval) {
        return; // Skip if interval hasn't elapsed
    }
    lastCheckTime = currentTime;
    
    checkPressure();
    checkWaterTemperature();
}

void SafetySystem::checkPressure() {
    PressureSensor* sensor = (PressureSensor*)SensorController::findSensorByName("pressureSensor");
    if (!sensor) {
        Logger::log(Logger::LogLevel::ERROR, "Pressure sensor not found in system");
        return;
    }

    if (!sensor->isHealthy()) {
        uint8_t errorFlags = sensor->getErrorFlags();
        
        if (errorFlags & static_cast<uint8_t>(PressureError::DISCONNECTED)) {
            Logger::log(Logger::LogLevel::ERROR, F("Pressure sensor disconnected or wire broken"));
        }
        if (errorFlags & static_cast<uint8_t>(PressureError::OVER_RANGE)) {
            Logger::log(Logger::LogLevel::ERROR, "Pressure sensor over range - possible short circuit");
        }
        if (errorFlags & static_cast<uint8_t>(PressureError::RAPID_CHANGE)) {
            Logger::log(Logger::LogLevel::WARNING, "Pressure changing too rapidly - possible sensor malfunction");
        }
        if (errorFlags & static_cast<uint8_t>(PressureError::OUT_OF_BOUNDS)) {
            Logger::log(Logger::LogLevel::WARNING, "Pressure out of valid range");
        }
        if (errorFlags & static_cast<uint8_t>(PressureError::ADC_ERROR)) {
            Logger::log(Logger::LogLevel::WARNING, "ADC reading error on pressure sensor");
        }
    }
}

void SafetySystem::checkWaterTemperature() {
    float temp = SensorController::readSensor("waterTempSensor");
    if (temp < MIN_WATER_TEMP) {
        Logger::log(Logger::LogLevel::WARNING, F("Water temperature low"));
    }
    if (temp > MAX_WATER_TEMP) {
        Logger::log(Logger::LogLevel::WARNING, F("Water temperature high"));
    }
    if (temp > CRITICAL_WATER_TEMP) {
        Logger::log(Logger::LogLevel::ERROR, F("Water temperature critical"));
        _stateMachine.stopAllPrograms();
    }
}

void SafetySystem::logAlert(const String& message, Logger::LogLevel level) {
    if ((level == Logger::LogLevel::ERROR && alarmEnabled) || 
        (level == Logger::LogLevel::WARNING && warningEnabled)) {
            
        unsigned long currentTime = millis();
        unsigned long seconds = currentTime / 1000;
        unsigned long minutes = seconds / 60;
        unsigned long hours = minutes / 60;
        
        String timeStamp = String("[") + String(hours % 24) + ":" + 
                          String(minutes % 60) + ":" + 
                          String(seconds % 60) + "] ";
                          
        Logger::log(level, timeStamp + message);
    }
}