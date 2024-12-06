// APIHandler.h - Gestion des requêtes API
#ifndef API_HANDLER_H
#define API_HANDLER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "DataProvider.h"
#include "MessageFormatter.h"

class APIHandler {
public:
    explicit APIHandler(DataProvider& provider);
    static String serializeData(const SensorData& data);
    void handleGETRequest(AsyncWebServerRequest *request);
    void handlePOSTRequest(AsyncWebServerRequest *request);
    
private:
    DataProvider& dataProvider;
};

#endif
