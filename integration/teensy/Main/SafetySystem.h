#ifndef SAFETY_SYSTEM_H
#define SAFETY_SYSTEM_H

#include <Arduino.h>
#include "Logger.h"
#include "SensorController.h"
#include "ActuatorController.h"

// Forward declaration
class StateMachine;

// Moved outside the class
struct HeatingMonitoringStatus {
    bool isMonitoring;
    unsigned long monitorStartTime;
    float initialTemp;
    static const unsigned long MONITOR_DURATION = 300000;  // 5 minutes
    static constexpr float MIN_TEMP_INCREASE = 0.5;   // Minimum expected increase
};

class SafetySystem {
public:
    SafetySystem(float totalVolume, float maxVolumePercent, float minVolume, StateMachine& stateMachine);
    void checkLimits();
    bool shouldStop() const { return stopRequired; }
    static void setLogLevel(LogLevel level);
    void parseCommand(const String& command);
    void setCheckInterval(unsigned long interval) { checkInterval = interval; }

private:
    StateMachine& stateMachine;
    float totalVolume;
    float maxVolumePercent;
    float minVolume;
    bool stopRequired;
    bool alarmEnabled;
    bool warningEnabled;
    unsigned long lastCheckTime;
    unsigned long checkInterval;
    HeatingMonitoringStatus heatingStatus;

    void checkWaterTemperature();
    void checkAirTemperature();
    void checkElectronicTemperature();
    void checkPH();
    void checkDissolvedOxygen();
    void checkVolume();
    void checkTurbidity();
    void checkHeatingEffectiveness();
    void logAlert(const String& message, LogLevel level);

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
    static constexpr float MAX_TURBIDITY = 1000.0;
};

#endif // SAFETY_SYSTEM_H