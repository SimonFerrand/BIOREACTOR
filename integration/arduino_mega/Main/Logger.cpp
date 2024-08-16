// Logger.cpp
#include "Logger.h"
#include <ArduinoJson.h>

#include "SensorController.h"
#include "ActuatorController.h"

LogLevel Logger::currentLevel = LogLevel::INFO;

void Logger::log(LogLevel level, const String& message) {
    if (level >= currentLevel) {
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
    // Read sensor values
    float wTemp = SensorController::readSensor("waterTempSensor");
    float aTemp = SensorController::readSensor("airTempSensor");
    float eTemp = SensorController::readSensor("electronicTempSensor");
    float pH = SensorController::readSensor("phSensor");
    float turb = SensorController::readSensor("turbiditySensorSEN0554");
    float oxy = SensorController::readSensor("oxygenSensor");
    float aflow = SensorController::readSensor("airFlowSensor");

    // Read actuator statuses
    bool apStat = ActuatorController::isActuatorRunning("airPump");
    bool dpStat = ActuatorController::isActuatorRunning("drainPump");
    bool spStat = ActuatorController::isActuatorRunning("samplePump");
    bool npStat = ActuatorController::isActuatorRunning("nutrientPump");
    bool bpStat = ActuatorController::isActuatorRunning("basePump");
    bool smStat = ActuatorController::isActuatorRunning("stirringMotor");
    bool hpStat = ActuatorController::isActuatorRunning("heatingPlate");
    bool lgStat = ActuatorController::isActuatorRunning("ledGrowLight");

    // Debug logs
    log(LogLevel::DEBUG, "Water Temp: " + String(wTemp));
    log(LogLevel::DEBUG, "Air Temp: " + String(aTemp));
    log(LogLevel::DEBUG, "Electronic Temp: " + String(eTemp));
    log(LogLevel::DEBUG, "pH: " + String(pH));
    log(LogLevel::DEBUG, "Turbidity: " + String(turb));
    log(LogLevel::DEBUG, "Oxygen: " + String(oxy));
    log(LogLevel::DEBUG, "Air Flow: " + String(aflow));

    // Construct JSON string manually
    String output = "{";
    output += "\"program\":\"" + currentProgram + "\",";
    output += "\"status\":\"" + programStatus + "\",";
    output += "\"airP\":" + String(apStat ? 1 : 0) + ",";
    output += "\"drainP\":" + String(dpStat ? 1 : 0) + ",";
    output += "\"sampleP\":" + String(spStat ? 1 : 0) + ",";
    output += "\"nutrientP\":" + String(npStat ? 1 : 0) + ",";
    output += "\"baseP\":" + String(bpStat ? 1 : 0) + ",";
    output += "\"stirringM\":" + String(smStat ? 1 : 0) + ",";
    output += "\"heatingP\":" + String(hpStat ? 1 : 0) + ",";
    output += "\"led\":" + String(lgStat ? 1 : 0) + ",";
    output += "\"waterTemp\":" + String(wTemp) + ",";
    output += "\"airTemp\":" + String(aTemp) + ",";
    output += "\"elecTemp\":" + String(eTemp) + ",";
    output += "\"pH\":" + String(pH) + ",";
    output += "\"turbidity\":" + String(turb) + ",";
    output += "\"oxygen\":" + String(oxy) + ",";
    output += "\"airFlow\":" + String(aflow);
    output += "}";

    // Print JSON string to serial
    Serial.println(output);
}

// void Logger::logProgramEvent
void Logger::logStartupParameters(const String& programType, int rateOrSpeed, int duration,
    float tempSetpoint, float phSetpoint, float doSetpoint, float nutrientConc,
    float baseConc, const String& experimentName, const String& comment) {
    
    StaticJsonDocument<256> doc;

    doc["ev"] = "startup";
    doc["pt"] = programType;
    doc["rate"] = rateOrSpeed;
    doc["dur"] = duration;
    doc["tSet"] = tempSetpoint;
    doc["phSet"] = phSetpoint;
    doc["doSet"] = doSetpoint;
    doc["nutC"] = nutrientConc;
    doc["baseC"] = baseConc;
    doc["expN"] = experimentName;
    doc["comm"] = comment;

    String output;
    serializeJson(doc, output);
    Serial.println(output);
}

void Logger::logPIDData(const String& pidType, float setpoint, float input, float output) {
    StaticJsonDocument<128> doc;
    
    doc["ev"] = "pid";
    doc["type"] = pidType;
    doc["set"] = setpoint;
    doc["in"] = input;
    doc["out"] = output;
    
    String jsonOutput;
    serializeJson(doc, jsonOutput);
    Serial.println(jsonOutput);
}

void Logger::setLogLevel(LogLevel level) {
    currentLevel = level;
}

void Logger::logSensorData() {
    log(LogLevel::INFO, "Water Temperature: " + String(SensorController::readSensor("waterTempSensor")) + " °C");
    log(LogLevel::INFO, "Air Temperature: " + String(SensorController::readSensor("airTempSensor")) + " °C");
    log(LogLevel::INFO, "Electronic Temperature: " + String(SensorController::readSensor("electronicTempSensor")) + " °C");
    log(LogLevel::INFO, "pH: " + String(SensorController::readSensor("phSensor")));
    log(LogLevel::INFO, "Turbidity: " + String(SensorController::readSensor("turbiditySensorSEN0554")) + " voltage");
    log(LogLevel::INFO, "Dissolved Oxygen: " + String(SensorController::readSensor("oxygenSensor")) + " mg/L");
    log(LogLevel::INFO, "Air Flow: " + String(SensorController::readSensor("airFlowSensor")) + " L/min");
}

void Logger::logActuatorData() {
    log(LogLevel::INFO, "Air Pump: " + String(ActuatorController::isActuatorRunning("airPump") ? "ON" : "OFF"));
    log(LogLevel::INFO, "Drain Pump: " + String(ActuatorController::isActuatorRunning("drainPump") ? "ON" : "OFF"));
    log(LogLevel::INFO, "Sample Pump: " + String(ActuatorController::isActuatorRunning("samplePump") ? "ON" : "OFF"));
    log(LogLevel::INFO, "Nutrient Pump: " + String(ActuatorController::isActuatorRunning("nutrientPump") ? "ON" : "OFF"));
    log(LogLevel::INFO, "Base Pump: " + String(ActuatorController::isActuatorRunning("basePump") ? "ON" : "OFF"));
    log(LogLevel::INFO, "Stirring Motor: " + String(ActuatorController::isActuatorRunning("stirringMotor") ? "ON" : "OFF"));
    log(LogLevel::INFO, "Heating Plate: " + String(ActuatorController::isActuatorRunning("heatingPlate") ? "ON" : "OFF"));
    log(LogLevel::INFO, "LED Grow Light: " + String(ActuatorController::isActuatorRunning("ledGrowLight") ? "ON" : "OFF"));
}