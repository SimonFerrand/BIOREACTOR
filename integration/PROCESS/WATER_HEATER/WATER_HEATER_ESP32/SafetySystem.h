// SafetySystem.h
#ifndef SAFETY_SYSTEM_H
#define SAFETY_SYSTEM_H

#include <Arduino.h>
#include "SensorController.h"
#include "PressureSensor.h"
#include "StateMachine.h"
#include "Logger.h"
#include "TaskManager.h"
#include "config.h"


class SafetySystem {
public:
    explicit SafetySystem(StateMachine& stateMachine);
    
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

    bool begin(); //begin Task


private:
    unsigned long lastCheckTime;     // Time of last safety check
    unsigned long checkInterval;     // Interval between safety checks
    bool alarmEnabled;              // Whether alarms are enabled
    bool warningEnabled;            // Whether warnings are enabled
    StateMachine& _stateMachine;

    void checkPressure();           // Check pressure sensor status
    void checkWaterTemperature();   // Check water temperature status
    void logSafetyEvent(const String& message, Logger::LogLevel level) {
      Logger::log(level, message);
    }
    void logAlert(const String& message, Logger::LogLevel level);

    // Task
    TaskHandle_t taskHandle;
    static void safetyTask(void* parameter);
    const TickType_t taskFrequency = pdMS_TO_TICKS(TASK_INTERVAL_SAFETY);
};

#endif // SAFETY_SYSTEM_H