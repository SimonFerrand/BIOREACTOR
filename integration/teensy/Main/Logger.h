// Logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "DataCollector.h"
#include "ProgramBase.h"

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
    static void logProgramEvent(const String& programName, ProgramBase* program);
    static void logPIDData(const String& pidType, float setpoint, float input, float output);
    static void setLogLevel(LogLevel level);
    static void logSensorData();
    static void logActuatorData();
    static void logVolumeData();
    static void logAllData(const String& currentProgram, int currentState);

private:
    static DataCollector* _dataCollector;
    static LogLevel _currentLevel;
};

#endif // LOGGER_H