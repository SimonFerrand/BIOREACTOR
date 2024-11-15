// SafetySystem.cpp
#include "SafetySystem.h"
#include "StateMachine.h"

SafetySystem::SafetySystem(float totalVolume, float maxVolumePercent, float minVolume, StateMachine& stateMachine)
    : stateMachine(stateMachine),          
      totalVolume(totalVolume),            
      maxVolumePercent(maxVolumePercent),  
      minVolume(minVolume),                
      alarmEnabled(true),
      warningEnabled(true),
      lastCheckTime(0),
      checkInterval(30000) {} // 30 seconds by default

void SafetySystem::checkLimits() {
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime < checkInterval) {
        return; // Do not check if the interval has not elapsed
    }
    lastCheckTime = currentTime;
    checkWaterTemperature();
    checkAirTemperature();
    checkElectronicTemperature();
    checkPH();
    checkDissolvedOxygen();
    checkVolume();
    checkTurbidity();
    checkHeatingEffectiveness(); 
}

void SafetySystem::checkWaterTemperature() {
    float temp = SensorController::readSensor("waterTempSensor");
    if (temp < MIN_WATER_TEMP) logAlert("Water temperature low", LogLevel::WARNING);
    if (temp > MAX_WATER_TEMP) logAlert("Water temperature high", LogLevel::WARNING);
    if (temp > CRITICAL_WATER_TEMP) {
        logAlert("Water temperature critical", LogLevel::ERROR);
        stateMachine.stopAllPrograms();
    }
}

void SafetySystem::checkAirTemperature() {
    float temp = SensorController::readSensor("airTempSensor");
    if (temp < MIN_AIR_TEMP) logAlert("Air temperature low", LogLevel::WARNING);
    if (temp > MAX_AIR_TEMP) logAlert("Air temperature high", LogLevel::WARNING);
}

void SafetySystem::checkPH() {
    float pH = SensorController::readSensor("phSensor");
    if (pH < MIN_PH) logAlert("pH low", LogLevel::WARNING);
    if (pH > MAX_PH) logAlert("pH high", LogLevel::WARNING);
    if (pH > CRITICAL_PH) {
        logAlert("pH critical", LogLevel::ERROR);
        //stateMachine.stopAllPrograms();
    }
}

void SafetySystem::checkDissolvedOxygen() {
    float do_percent = SensorController::readSensor("oxygenSensor");
    if (do_percent < MIN_DO) logAlert("Dissolved oxygen low", LogLevel::WARNING);
}

void SafetySystem::checkVolume() {
    // Note: You'll need to implement a method to get the current volume
    float volume = 0; // Replace with actual method to get current volume
    if (volume <= minVolume) {
        logAlert("Volume below minimum", LogLevel::WARNING);
    }
    if (volume >= maxVolumePercent * totalVolume) {
        logAlert("Volume near maximum", LogLevel::ERROR);
        stateMachine.stopAllPrograms();
    }
}

void SafetySystem::checkTurbidity() {
    float turbidity = SensorController::readSensor("turbiditySensorSEN0554");
    if (turbidity > MAX_TURBIDITY) {
        logAlert("Turbidity high", LogLevel::WARNING);
    }
}

void SafetySystem::checkElectronicTemperature() {
    float temp = SensorController::readSensor("electronicTempSensor");
    if (temp > MAX_ELECTRONIC_TEMP) {
        logAlert("Electronic temperature critical", LogLevel::ERROR);
    } else if (temp > MAX_ELECTRONIC_TEMP - 10) {  // Warning at 5°C below the limit
        logAlert("Electronic temperature high", LogLevel::WARNING);
    }
}

void SafetySystem::logAlert(const String& message, LogLevel level) {
    if ((level == LogLevel::ERROR && alarmEnabled) || 
        (level == LogLevel::WARNING && warningEnabled)) {
        Logger::log(level, message);
    }
}

void SafetySystem::parseCommand(const String& command) {
    if (command.startsWith("warnings ")) {
        String value = command.substring(8);
        warningEnabled = (value == "true");
        Logger::log(LogLevel::INFO, "Warnings set to " + String(warningEnabled ? "enabled" : "disabled"));
    } else if (command.startsWith("alarms ")) {
        String value = command.substring(6);
        alarmEnabled = (value == "true");
        Logger::log(LogLevel::INFO, "Alarms set to " + String(alarmEnabled ? "enabled" : "disabled"));
    } else {
        Logger::log(LogLevel::WARNING, "Unknown command: " + command);
    }
}

void SafetySystem::checkHeatingEffectiveness() {
    float currentTemp = SensorController::readSensor("waterTempSensor");
    
    HeatingPlate* heatingPlate = (HeatingPlate*)ActuatorController::findActuatorByName("heatingPlate");
    if (heatingPlate->isOn()) {
        if (!heatingStatus.isMonitoring) {
            // Start new monitoring period
            heatingStatus.isMonitoring = true;
            heatingStatus.monitorStartTime = millis();
            heatingStatus.initialTemp = currentTemp;
            Logger::log(LogLevel::INFO, "Started heating monitoring at temperature: " + String(currentTemp));
        }

        unsigned long monitoringDuration = millis() - heatingStatus.monitorStartTime;
        if (monitoringDuration >= HeatingMonitoringStatus::MONITOR_DURATION) {  // Changed here
            float tempChange = currentTemp - heatingStatus.initialTemp;
            if (tempChange < HeatingMonitoringStatus::MIN_TEMP_INCREASE) {      // Changed here
                logAlert("Temperature not increasing while heating - possible sensor malfunction", LogLevel::ERROR);
                Logger::log(LogLevel::ERROR, "Initial temp: " + String(heatingStatus.initialTemp) + 
                                           ", Current temp: " + String(currentTemp) + 
                                           ", Change: " + String(tempChange));
                stateMachine.stopAllPrograms();
            }
            heatingStatus.isMonitoring = false;
        }
    } else {
        heatingStatus.isMonitoring = false;
    }
}