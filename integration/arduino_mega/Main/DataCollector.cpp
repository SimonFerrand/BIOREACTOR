// DataCollector.cpp
#include "DataCollector.h"

DataCollector::DataCollector(VolumeManager& volumeManager) : _volumeManager(volumeManager) {}

String DataCollector::collectProgramEvent(const String& programName, const String& status, 
                                          float tempSetpoint, float phSetpoint, float doSetpoint,
                                          float nutrientConc, float baseConc, int duration,
                                          const String& experimentName, const String& comment) {
    JsonDocument doc;

    doc["ev"] = "program";
    doc["program"] = programName;
    doc["status"] = status;
    doc["tempSetpoint"] = tempSetpoint;
    doc["phSetpoint"] = phSetpoint;
    doc["doSetpoint"] = doSetpoint;
    doc["nutrientConc"] = nutrientConc;
    doc["baseConc"] = baseConc;
    doc["duration"] = duration;
    doc["experimentName"] = experimentName;
    doc["comment"] = comment;

    String output;
    serializeJson(doc, output);
    return output;
}

String DataCollector::collectSensorData() {
    JsonDocument doc;

    doc["waterTemp"] = SensorController::readSensor("waterTempSensor");
    doc["airTemp"] = SensorController::readSensor("airTempSensor");
    doc["elecTemp"] = SensorController::readSensor("electronicTempSensor");
    doc["pH"] = SensorController::readSensor("phSensor");
    doc["turbidity"] = SensorController::readSensor("turbiditySensorSEN0554");
    doc["oxygen"] = SensorController::readSensor("oxygenSensor");
    doc["airFlow"] = SensorController::readSensor("airFlowSensor");

    String output;
    serializeJson(doc, output);
    return output;
}

String DataCollector::collectActuatorData() {
    JsonDocument doc;

    doc["airPump"] = ActuatorController::isActuatorRunning("airPump");
    doc["drainPump"] = ActuatorController::isActuatorRunning("drainPump");
    doc["samplePump"] = ActuatorController::isActuatorRunning("samplePump");
    doc["nutrientPump"] = ActuatorController::isActuatorRunning("nutrientPump");
    doc["basePump"] = ActuatorController::isActuatorRunning("basePump");
    doc["stirringMotor"] = ActuatorController::isActuatorRunning("stirringMotor");
    doc["heatingPlate"] = ActuatorController::isActuatorRunning("heatingPlate");
    doc["ledGrowLight"] = ActuatorController::isActuatorRunning("ledGrowLight");

    String output;
    serializeJson(doc, output);
    return output;
}

String DataCollector::collectVolumeData() {
    JsonDocument doc;

    doc["currentVolume"] = _volumeManager.getCurrentVolume();
    doc["availableVolume"] = _volumeManager.getAvailableVolume();
    doc["addedNaOH"] = _volumeManager.getAddedNaOH();
    doc["addedNutrient"] = _volumeManager.getAddedNutrient();
    doc["addedMicroalgae"] = _volumeManager.getAddedMicroalgae();
    doc["removedVolume"] = _volumeManager.getRemovedVolume();

    String output;
    serializeJson(doc, output);
    return output;
}

String DataCollector::collectPIDData(const String& pidType, float setpoint, float input, float output) {
    JsonDocument doc;
    
    doc["ev"] = "pid";
    doc["type"] = pidType;
    doc["set"] = setpoint;
    doc["in"] = input;
    doc["out"] = output;
    
    String jsonOutput;
    serializeJson(doc, jsonOutput);
    return jsonOutput;
}