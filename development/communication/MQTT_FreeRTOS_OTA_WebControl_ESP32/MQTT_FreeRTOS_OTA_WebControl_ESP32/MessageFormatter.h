// ===== MessageFormatter.h =====
#ifndef MESSAGE_FORMATTER_H
#define MESSAGE_FORMATTER_H

#include <ArduinoJson.h>
#include "config.h"
#include <WiFi.h>
#include "Logger.h"

struct SensorData {
    float waterTemp;
    float airTemp;
    float pH;
    float oxygen;
    float turbidity;
};

class MessageFormatter {
public:
    static JsonDocument createSensorMessage(const SensorData& data);
    static JsonDocument createStatusMessage(const String& status);
    static JsonDocument createHeartbeatMessage();
    static JsonDocument createErrorMessage(const String& error);
    
    static bool parseSensorCommand(const String& message, SensorData& data);
    static bool parseCommand(const String& message, String& command, JsonDocument& params);

private:
    static void addCommonFields(JsonDocument& doc);
    static uint32_t getTimestamp();
};

#endif