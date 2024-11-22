// SafetySystem.cpp
#include "SafetySystem.h"

SafetySystem::SafetySystem()
    : lastCheckTime(0)
    , checkInterval(1000)  // Check every second by default
    , alarmEnabled(true)
    , warningEnabled(true)
{}

void SafetySystem::checkLimits() {
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime < checkInterval) {
        return; // Skip if interval hasn't elapsed
    }
    lastCheckTime = currentTime;
    
    checkPressure();
}

void SafetySystem::checkPressure() {
    // Try to get pressure sensor
    PressureSensor* sensor = (PressureSensor*)SensorController::findSensorByName("pressureSensor");
    if (!sensor) {
        logAlert("Pressure sensor not found in system", AlertLevel::ERROR);
        return;
    }

    // Check sensor health
    if (!sensor->isHealthy()) {
        uint8_t errorFlags = sensor->getErrorFlags();
        
        // Check each error condition
        if (errorFlags & static_cast<uint8_t>(PressureError::DISCONNECTED)) {
            logAlert("Pressure sensor disconnected or wire broken", AlertLevel::ERROR);
        }
        if (errorFlags & static_cast<uint8_t>(PressureError::OVER_RANGE)) {
            logAlert("Pressure sensor over range - possible short circuit", AlertLevel::ERROR);
        }
        if (errorFlags & static_cast<uint8_t>(PressureError::RAPID_CHANGE)) {
            logAlert("Pressure changing too rapidly - possible sensor malfunction", AlertLevel::WARNING);
        }
        if (errorFlags & static_cast<uint8_t>(PressureError::OUT_OF_BOUNDS)) {
            logAlert("Pressure out of valid range", AlertLevel::ERROR);
        }
        if (errorFlags & static_cast<uint8_t>(PressureError::ADC_ERROR)) {
            logAlert("ADC reading error on pressure sensor", AlertLevel::ERROR);
        }
    }
}

void SafetySystem::logAlert(const String& message, AlertLevel level) {
    if ((level == AlertLevel::ERROR && alarmEnabled) || 
        (level == AlertLevel::WARNING && warningEnabled)) {
        // Format timestamp
        unsigned long currentTime = millis();
        unsigned long seconds = currentTime / 1000;
        unsigned long minutes = seconds / 60;
        unsigned long hours = minutes / 60;
        
        // Create timestamp string
        char timestamp[20];
        sprintf(timestamp, "[%02lu:%02lu:%02lu]", 
                hours % 24, minutes % 60, seconds % 60);
        
        // Print alert with timestamp
        Serial.print(timestamp);
        Serial.print(" ");
        Serial.print(level == AlertLevel::ERROR ? "ERROR: " : "WARNING: ");
        Serial.println(message);
    }
}