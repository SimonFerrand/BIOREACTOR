#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "Logger.h"
#include "config.h"

class WiFiManager {
public:
    static void initialize();
    static void connect();
    static void disconnect();
    static bool isConnected();
    static int8_t getSignalStrength();

private:
    static bool initialized;
    static bool connected;
    static void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);
};

#endif