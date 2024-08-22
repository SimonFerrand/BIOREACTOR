// Logger.cpp
#include "Logger.h"

DataCollector* Logger::_dataCollector = nullptr;
LogLevel Logger::_currentLevel = LogLevel::INFO;

void Logger::initialize(DataCollector& dataCollector) {
    _dataCollector = &dataCollector;
}

void Logger::log(LogLevel level, const String& message) {
    if (level >= _currentLevel) {
        String prefix;
        switch (level) {
            case LogLevel::DEBUG: prefix = "DEBUG: "; break;
            case LogLevel::INFO: prefix = "INFO: "; break;
            case LogLevel::WARNING: prefix = "WARNING: "; break;
            case LogLevel::ERROR: prefix = "ERROR: "; break;
        }
        Serial.println(prefix + message);
    }
}

void Logger::logData(const String& currentProgram, const String& programStatus) {
    if (_dataCollector) {
        String sensorData = _dataCollector->collectSensorData();
        String actuatorData = _dataCollector->collectActuatorData();
        
        log(LogLevel::INFO, "Program: " + currentProgram + ", Status: " + programStatus);
        log(LogLevel::INFO, "Sensor Data: " + sensorData);
        log(LogLevel::INFO, "Actuator Data: " + actuatorData);
        Serial.println();
    }
}

void Logger::logPIDData(const String& pidType, float setpoint, float input, float output) {
    if (_dataCollector) {
        String pidData = _dataCollector->collectPIDData(pidType, setpoint, input, output);
        log(LogLevel::INFO, "PID Data: " + pidData);
    }
}

void Logger::setLogLevel(LogLevel level) {
    _currentLevel = level;
}

void Logger::logSensorData() {
    if (_dataCollector) {
        String sensorData = _dataCollector->collectSensorData();
        log(LogLevel::INFO, "Sensor Data: " + sensorData);
    }
}

void Logger::logActuatorData() {
    if (_dataCollector) {
        String actuatorData = _dataCollector->collectActuatorData();
        log(LogLevel::INFO, "Actuator Data: " + actuatorData);
    }
}

void Logger::logVolumeData() {
    if (_dataCollector) {
        String volumeData = _dataCollector->collectVolumeData();
        log(LogLevel::INFO, "Volume Data: " + volumeData);
    }
}

void Logger::logAllData(const String& currentProgram, int currentState) {
    if (_dataCollector) {
        String allData = _dataCollector->collectAllData(currentProgram, currentState);
        log(LogLevel::INFO, "Periodic Event :" + allData);
    } else {
        log(LogLevel::ERROR, "DataCollector not initialized");
    }
}

void Logger::logProgramEvent(const String& programName, ProgramBase* program) {
    if (_dataCollector) {
        String eventData = _dataCollector->collectProgramEvent(programName, program);
        log(LogLevel::INFO, "Program Event: " + eventData);
    }
}