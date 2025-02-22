// DataCollector.cpp
#include "DataCollector.h"

DataCollector::DataCollector(VolumeManager& volumeManager) : _volumeManager(volumeManager) {}

String DataCollector::collectProgramEvent(const String& programName, ProgramBase* program) {
    JsonDocument doc;

    doc["program"] = programName;

    if (program != nullptr) {
        program->getParameters(doc);
    }

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
    doc["fillPump"] = ActuatorController::isActuatorRunning("fillPump");
    doc["stirringMotor"] = ActuatorController::isActuatorRunning("stirringMotor");
    doc["heatingPlate"] = ActuatorController::isActuatorRunning("heatingPlate");
    doc["ledGrowLight"] = ActuatorController::isActuatorRunning("ledGrowLight");

    String output;
    serializeJson(doc, output);
    return output;
}

String DataCollector::collectActuatorSetpoints() {
    JsonDocument doc;
    
    doc["airPumpValue"] = ActuatorController::getCurrentValue("airPump");
    doc["drainPumpValue"] = ActuatorController::getCurrentValue("drainPump");
    doc["samplePumpValue"] = ActuatorController::getCurrentValue("samplePump");
    doc["nutrientPumpValue"] = ActuatorController::getCurrentValue("nutrientPump");
    doc["basePumpValue"] = ActuatorController::getCurrentValue("basePump");
    doc["fillPumpValue"] = ActuatorController::getCurrentValue("fillPump");
    doc["stirringMotorValue"] = ActuatorController::getCurrentValue("stirringMotor");
    doc["heatingPlateValue"] = ActuatorController::getCurrentValue("heatingPlate");
    doc["ledGrowLightValue"] = ActuatorController::getCurrentValue("ledGrowLight");
    
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
    
    doc["type"] = pidType;
    doc["set"] = setpoint;
    doc["in"] = input;
    doc["out"] = output;
    
    String jsonOutput;
    serializeJson(doc, jsonOutput);
    return jsonOutput;
}

/*
String DataCollector::collectAllData(const String& currentProgram, int currentState) {
    JsonDocument doc;

    // Add program information
    JsonObject StateData = doc.createNestedObject("StateData");
    doc["currentProgram"] = currentProgram;
    doc["programState"] = static_cast<int>(currentState);

    // Collect sensor data
    JsonObject sensorData = doc.createNestedObject("sensorData");
    String sensorDataStr = collectSensorData();
    deserializeJson(sensorData, sensorDataStr);

    // Collect actuator data
    JsonObject actuatorData = doc.createNestedObject("actuatorData");
    String actuatorDataStr = collectActuatorData();
    deserializeJson(actuatorData, actuatorDataStr);

    // Collect actuator setpoints
    JsonObject actuatorSetpoints = doc.createNestedObject("actuatorSetpoints");
    String setpointsDataStr = collectActuatorSetpoints();
    deserializeJson(actuatorSetpoints, setpointsDataStr);

    // Collect volume data
    JsonObject volumeData = doc.createNestedObject("volumeData");
    String volumeDataStr = collectVolumeData();
    deserializeJson(volumeData, volumeDataStr);

    String output;
    serializeJson(doc, output);
    return output;
}
*/

String DataCollector::collectAllData(const String& currentProgram, int currentState) {
    JsonDocument doc;
    JsonDocument tempDoc;

    // Add program information
    doc["currentProgram"] = currentProgram;
    doc["programState"] = static_cast<int>(currentState);

    // Collect sensor data
    String sensorDataStr = collectSensorData();
    deserializeJson(tempDoc, sensorDataStr);
    doc["sensorData"] = tempDoc;
    tempDoc.clear();

    // Collect actuator data
    String actuatorDataStr = collectActuatorData();
    deserializeJson(tempDoc, actuatorDataStr);
    doc["actuatorData"] = tempDoc;
    tempDoc.clear();

    // Collect actuator setpoints
    String setpointsDataStr = collectActuatorSetpoints();
    deserializeJson(tempDoc, setpointsDataStr);
    doc["actuatorSetpoints"] = tempDoc;
    tempDoc.clear();

    // Collect volume data
    String volumeDataStr = collectVolumeData();
    deserializeJson(tempDoc, volumeDataStr);
    doc["volumeData"] = tempDoc;
    tempDoc.clear();

    String output;
    serializeJson(doc, output);
    return output;
}
