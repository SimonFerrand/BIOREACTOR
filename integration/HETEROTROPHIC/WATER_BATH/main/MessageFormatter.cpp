// ===== MessageFormatter.cpp =====
#include "MessageFormatter.h"
#include <Arduino.h>

JsonDocument MessageFormatter::createSensorMessage(const SensorData& data) {
    JsonDocument doc;
    
    JsonObject sensorData = doc["sensorData"].to<JsonObject>();
    sensorData["waterTemp"] = data.waterTemp;
    
    addCommonFields(doc);
    return doc;
}

JsonDocument MessageFormatter::createStatusMessage(const String& status) {
    JsonDocument doc;
    doc["status"] = status;
    doc["version"] = "1.0.0";
    
    addCommonFields(doc);
    return doc;
}

JsonDocument MessageFormatter::createHeartbeatMessage() {
    JsonDocument doc;
    doc["type"] = "heartbeat";
    doc["uptime"] = millis() / 1000;

    auto system = doc["system"].to<JsonObject>();
    system["heap"] = ESP.getFreeHeap();
    system["wifi_strength"] = WiFi.RSSI();

    addCommonFields(doc);
    return doc;
}

JsonDocument MessageFormatter::createErrorMessage(const String& error) {
    JsonDocument doc;
    doc["error"] = error;
    doc["severity"] = "error";
    
    addCommonFields(doc);
    return doc;
}

bool MessageFormatter::parseSensorCommand(const String& message, SensorData& data) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        Logger::log(Logger::LogLevel::ERROR, "Failed to parse sensor command: " + String(error.c_str()));
        return false;
    }
    
    // Vérification de la présence des champs requis
    if (!doc["sensorData"].is<JsonObject>()) {
        Logger::log(Logger::LogLevel::ERROR, "Missing sensorData object in command");
        return false;
    }
    
    JsonObject sensorData = doc["sensorData"];
    data.waterTemp = sensorData["waterTemp"];
    
    return true;
}

bool MessageFormatter::parseCommand(const String& message, String& command, JsonDocument& params) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Logger::log(Logger::LogLevel::ERROR, "Failed to parse command: " + String(error.c_str()));
        return false;
    }

    if (!doc["command"].is<const char*>()) {
        Logger::log(Logger::LogLevel::ERROR, "Missing command field");
        return false;
    }

    command = doc["command"].as<String>();

    if (doc["params"].is<JsonObject>()) {
        params = doc["params"];
    }

    return true;
}

void MessageFormatter::addCommonFields(JsonDocument& doc) {
    doc["deviceId"] = MQTT_CLIENT_ID;
    doc["timestamp"] = getTimestamp();
}

uint32_t MessageFormatter::getTimestamp() {
    return myTZ.now(); // myTZ.dateTime("H:i:s")
}