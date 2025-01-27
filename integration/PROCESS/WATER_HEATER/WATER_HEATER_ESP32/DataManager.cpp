// DataManager.cpp
#include "DataManager.h"
#include "StateMachine.h" // to avoid circular dependencies 

SensorData DataManager::collectSensorData() {
    SensorData data;
    data.waterTemp = SensorController::readSensor("waterTempSensor");
    data.pressure = SensorController::readSensor("pressureSensor");
    return data;
}

PressureStats DataManager::collectPressureStats() {
    PressureSensor* sensor = (PressureSensor*)SensorController::findSensorByName("pressureSensor");
    if (sensor) {
        return sensor->getStatistics();
    }
    return PressureStats{};
}

String DataManager::collectActuatorState() {
    JsonDocument doc;
    doc["heatingPlate"] = ActuatorController::isActuatorRunning("heatingPlate");
    String output;
    serializeJson(doc, output);
    return output;
}

String DataManager::collectActuatorValues() {
    JsonDocument doc;
    doc["heatingPlateValue"] = ActuatorController::getCurrentValue("heatingPlate");
    String output;
    serializeJson(doc, output);
    return output;
}

String DataManager::collectDeviceInfo() {
    JsonDocument doc;
    doc["deviceID"] = MQTT_CLIENT_ID;
    doc["ip"] = WiFi.localIP().toString();
    doc["uptime"] = millis() / 1000;
    String output;
    serializeJson(doc, output);
    return output;
}

String DataManager::collectSystemMetrics() {
    JsonDocument doc;
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["wifiStrength"] = WiFi.RSSI();
    String output;
    serializeJson(doc, output);
    return output;
}

String DataManager::collectProgramState(const String& programName, ProgramBase* program) {
    JsonDocument doc;
    doc["program"] = programName;
    if (program != nullptr) {
        program->getParameters(doc);
    }
    String output;
    serializeJson(doc, output);
    return output;
}

String DataManager::collectAllData(const StateMachine& stateMachine) {
    JsonDocument doc;
    
    // Programme et état
    doc["program"] = stateMachine.getCurrentProgram();
    doc["state"] = static_cast<int>(stateMachine.getCurrentState());
    
    // Ajouter toutes les données
    addSensorDataToJson(doc);
    addActuatorDataToJson(doc);
    addSystemDataToJson(doc);
    
    String output;
    serializeJson(doc, output);
    return output;
}

void DataManager::addSensorDataToJson(JsonDocument& doc) {
    SensorData data = collectSensorData();
    doc["sensorData"]["waterTemp"] = data.waterTemp;
    doc["sensorData"]["pressure"] = data.pressure;
}

void DataManager::addActuatorDataToJson(JsonDocument& doc) {
    doc["actuatorData"]["heatingPlate"] = ActuatorController::isActuatorRunning("heatingPlate");
    doc["actuatorValues"]["heatingPlateValue"] = ActuatorController::getCurrentValue("heatingPlate");
}

void DataManager::addSystemDataToJson(JsonDocument& doc) {
    doc["deviceInfo"]["id"] = MQTT_CLIENT_ID;
    doc["deviceInfo"]["ip"] = WiFi.localIP().toString();
    doc["deviceInfo"]["uptime"] = millis() / 1000;
    doc["systemMetrics"]["freeHeap"] = ESP.getFreeHeap();
    doc["systemMetrics"]["wifiStrength"] = WiFi.RSSI();
}

String DataManager::createHeartbeatMessage() {
    JsonDocument doc;
    doc["type"] = "heartbeat";
    doc["uptime"] = millis() / 1000;
    doc["heap"] = ESP.getFreeHeap();
    doc["wifi_strength"] = WiFi.RSSI();
    
    String output;
    serializeJson(doc, output);
    return output;
}

String DataManager::createErrorMessage(const String& error) {
    JsonDocument doc;
    doc["error"] = error;
    doc["severity"] = "error";
    
    String output;
    serializeJson(doc, output);
    return output;
}

// non utilisé? A supprimer?
bool DataManager::parseSensorCommand(const String& message, SensorData& data) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        return false;
    }
    
    if (!doc["sensorData"].is<JsonObject>()) {
        return false;
    }
    
    JsonObject sensorData = doc["sensorData"];
    data.waterTemp = sensorData["waterTemp"] | 0.0f;
    data.pressure = sensorData["pressure"] | 0.0f;
    
    return true;
}

// non utilisé? A supprimer?
bool DataManager::parseCommand(const String& message, String& command, JsonDocument& params) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        return false;
    }

    if (!doc["command"].is<const char*>()) {
        return false;
    }

    command = doc["command"].as<String>();
    if (doc["params"].is<JsonObject>()) {
        params = doc["params"];
    }

    return true;
}

void DataManager::addProgramDataToJson(JsonDocument& doc, const StateMachine& stateMachine) {
    // Ajoute l'état du programme
    doc["currentProgram"] = stateMachine.getCurrentProgram();
    doc["programState"] = static_cast<int>(stateMachine.getCurrentState());

    // Si un programme est en cours, ajoutez ses paramètres
    String programName = stateMachine.getCurrentProgram();
    if (programName != "None") {
        JsonObject programData = doc["programData"].to<JsonObject>();
        programData["name"] = programName;
        programData["state"] = static_cast<int>(stateMachine.getCurrentState());
        
        // Les paramètres spécifiques au programme seront ajoutés 
        // par la méthode getParameters() de chaque programme
    }
}

