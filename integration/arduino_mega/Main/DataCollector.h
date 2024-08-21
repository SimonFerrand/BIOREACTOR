// DataCollector.h
#ifndef DATA_COLLECTOR_H
#define DATA_COLLECTOR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "SensorController.h"
#include "ActuatorController.h"
#include "VolumeManager.h"

class DataCollector {
public:
    // Constructor that takes a reference to VolumeManager
    DataCollector(VolumeManager& volumeManager);

    // Collect program event data
    String collectProgramEvent(const String& programName, const String& status, 
                               float tempSetpoint, float phSetpoint, float doSetpoint,
                               float nutrientConc, float baseConc, int duration,
                               const String& experimentName, const String& comment);

    // Collect sensor data
    String collectSensorData();

    // Collect actuator data
    String collectActuatorData();

    // Collect volume data
    String collectVolumeData();

    // Collect PID data
    String collectPIDData(const String& pidType, float setpoint, float input, float output);

private:
    VolumeManager& _volumeManager;
};

#endif // DATA_COLLECTOR_H