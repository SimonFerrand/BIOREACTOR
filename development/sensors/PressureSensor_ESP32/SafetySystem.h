// SafetySystem.h
#ifndef SAFETY_SYSTEM_H
#define SAFETY_SYSTEM_H

#include <Arduino.h>
#include "SensorController.h"
#include "PressureSensor.h"

// Alert levels for safety system
enum class AlertLevel {
    WARNING,
    ERROR
};

class SafetySystem {
public:
    /**
     * Constructor
     */
    SafetySystem();
    
    /**
     * Check all safety limits
     * Should be called regularly in the main loop
     */
    void checkLimits();
    
    /**
     * Set the interval between safety checks
     * @param interval Time in milliseconds between checks
     */
    void setCheckInterval(unsigned long interval) { checkInterval = interval; }
    
    /**
     * Enable or disable warnings
     * @param enable True to enable warnings, false to disable
     */
    void enableWarnings(bool enable) { warningEnabled = enable; }
    
    /**
     * Enable or disable alarms
     * @param enable True to enable alarms, false to disable
     */
    void enableAlarms(bool enable) { alarmEnabled = enable; }

private:
    unsigned long lastCheckTime;     // Time of last safety check
    unsigned long checkInterval;     // Interval between safety checks
    bool alarmEnabled;              // Whether alarms are enabled
    bool warningEnabled;            // Whether warnings are enabled

    void checkPressure();           // Check pressure sensor status
    void logAlert(const String& message, AlertLevel level);  // Log safety alerts
};

#endif // SAFETY_SYSTEM_H