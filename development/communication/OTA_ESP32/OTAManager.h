// OTAManager.h
#pragma once

#include <WebServer.h> //#include <ESP8266WebServer.h>
#include <WiFi.h>
#include <ElegantOTA.h>
#include "config.h"

class OTAManager {
private:
    WebServer* server;
    unsigned long lastFailedAttempt;
    int failedAttempts;
    String allowedIP;
    
    void setupCallbacks();

public:
    OTAManager(WebServer* server);
    void begin();
    void loop();
};