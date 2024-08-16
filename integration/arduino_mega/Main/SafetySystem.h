// SafetySystem.h
#ifndef SAFETY_SYSTEM_H
#define SAFETY_SYSTEM_H

#include <Arduino.h>
#include "Logger.h"
#include "SensorController.h"
#include "ActuatorController.h"

class SafetySystem {
public:
    SafetySystem(float totalVolume, float maxVolumePercent, float minVolume);
    void checkLimits();
    bool shouldStop() const { return stopRequired; }
    void setLogger(Logger* logger) { this->logger = logger; } //This allows the SafetySystem to use the same logger as the rest of this application, ensuring consistent logging.
    void parseCommand(const String& command);
    void setCheckInterval(unsigned long interval) { checkInterval = interval; }

private:
    void checkWaterTemperature();
    void checkAirTemperature();
    void checkElectronicTemperature();
    void checkPH();
    void checkDissolvedOxygen();
    void checkVolume();
    void checkTurbidity();

    void logAlert(const String& message, LogLevel level);

    float totalVolume;
    float maxVolumePercent;
    float minVolume;
    bool stopRequired;
    Logger* logger;

    bool alarmEnabled;
    bool warningEnabled;
    unsigned long lastCheckTime;
    unsigned long checkInterval;

    // Safety thresholds
    static constexpr float MIN_WATER_TEMP = 15.0;
    static constexpr float MAX_WATER_TEMP = 45.0;
    static constexpr float CRITICAL_WATER_TEMP = 50.0;
    static constexpr float MIN_AIR_TEMP = 10.0;
    static constexpr float MAX_AIR_TEMP = 45.0;
    static constexpr float MAX_ELECTRONIC_TEMP = 60.0;
    static constexpr float MIN_PH = 2.0;
    static constexpr float MAX_PH = 12.0;
    static constexpr float CRITICAL_PH = 10.0;
    static constexpr float MIN_DO = 0.0;
    static constexpr float MAX_TURBIDITY = 1000.0; // Example value, adjust as needed
};

#endif // SAFETY_SYSTEM_H