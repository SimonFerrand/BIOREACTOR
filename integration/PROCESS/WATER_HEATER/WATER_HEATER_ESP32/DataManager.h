// DataManager.h
#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "SensorController.h"
#include "ActuatorController.h"
#include "ProgramBase.h"

struct SensorData {
    float waterTemp;
    float pressure;
};

// Forward declarations
class StateMachine;

class DataManager {
public:
    // Données des capteurs
    static SensorData collectSensorData();
    static PressureStats collectPressureStats();
    
    // État des actionneurs
    static String collectActuatorState();
    static String collectActuatorValues();
    
    // Informations système
    static String collectDeviceInfo(); // Reprend getDeviceInfo de DataProvider (IP, uptime, etc.)
    static String collectSystemMetrics(); // Mémoire, CPU, etc.

    // États des programmes
    static String collectProgramState(const String& programName, ProgramBase* program);
    
    // Collection complète
    static String collectAllData(const StateMachine& stateMachine);

    //
    static String createHeartbeatMessage();
    static String createErrorMessage(const String& error);
    static bool parseSensorCommand(const String& message, SensorData& data);
    static bool parseCommand(const String& message, String& command, JsonDocument& params);


private:
    static void addSensorDataToJson(JsonDocument& doc);
    static void addActuatorDataToJson(JsonDocument& doc);
    static void addSystemDataToJson(JsonDocument& doc);
    static void addProgramDataToJson(JsonDocument& doc, const StateMachine& stateMachine);
};

#endif