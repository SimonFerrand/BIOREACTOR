// APIHandler.cpp
#include "APIHandler.h"

APIHandler::APIHandler(DataProvider& provider) : dataProvider(provider) {}

String APIHandler::serializeData(const SensorData& data) {
    JsonDocument doc;
    doc["waterTemp"] = data.waterTemp;
    
    String json;
    serializeJson(doc, json);
    return json;
}