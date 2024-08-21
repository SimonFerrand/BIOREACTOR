// Logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "DataCollector.h"

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
public:
    static void initialize(DataCollector& dataCollector);
    static void log(LogLevel level, const String& message);
    static void logData(const String& currentProgram, const String& programStatus);
    static void logProgramEvent(const String& programName, const String& status, 
                                float tempSetpoint, float phSetpoint, float doSetpoint,
                                float nutrientConc, float baseConc, int duration,
                                const String& experimentName, const String& comment);
    static void logPIDData(const String& pidType, float setpoint, float input, float output);
    static void setLogLevel(LogLevel level);
    static void logSensorData();
    static void logActuatorData();
    static void logVolumeData();

private:
    static DataCollector* _dataCollector;
    static LogLevel _currentLevel;
};

#endif // LOGGER_H