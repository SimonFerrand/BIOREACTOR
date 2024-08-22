// DataCollector.h
#ifndef DATA_COLLECTOR_H
#define DATA_COLLECTOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "SensorController.h"
#include "ActuatorController.h"
#include "VolumeManager.h"
#include "ProgramBase.h"

class DataCollector {
public:
    // Constructor that takes a reference to VolumeManager
    DataCollector(VolumeManager& volumeManager);

    // Collect program event data
    String collectProgramEvent(const String& programName, ProgramBase* program);

    // Collect sensor data
    String collectSensorData();

    // Collect actuator data
    String collectActuatorData();

    // Collect volume data
    String collectVolumeData();

    // Collect PID data
    String collectPIDData(const String& pidType, float setpoint, float input, float output);

    // Colelct sensor, actuator and volume data
    String collectAllData(const String& currentProgram, int currentState);

private:
    VolumeManager& _volumeManager;
};

#endif // DATA_COLLECTOR_H