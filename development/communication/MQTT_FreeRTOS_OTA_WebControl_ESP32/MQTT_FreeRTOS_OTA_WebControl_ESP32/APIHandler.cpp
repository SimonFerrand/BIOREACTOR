// APIHandler.cpp
#include "APIHandler.h"

APIHandler::APIHandler(DataProvider& provider) : dataProvider(provider) {}

String APIHandler::serializeData(const SensorData& data) {
    JsonDocument doc;
    doc["waterTemp"] = data.waterTemp;
    doc["airTemp"] = data.airTemp;
    doc["pH"] = data.pH;
    doc["oxygen"] = data.oxygen;
    doc["turbidity"] = data.turbidity;
    
    String json;
    serializeJson(doc, json);
    return json;
}