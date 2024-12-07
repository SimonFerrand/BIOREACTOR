// WebServerManager.h
#ifndef WEBSERVER_MANAGER_H
#define WEBSERVER_MANAGER_H

#include <WebServer.h>
#include <ElegantOTA.h>
#include <Arduino.h>
#include "DataProvider.h"
#include "config.h"
#include "Logger.h"
#include "WebPageBuilder.h"
#include "APIHandler.h"

class WebServerManager {
public:
    explicit WebServerManager(DataProvider& dataProvider);
    
    void begin();
    void handle();  // Called from task
    void stop();

private:
    WebServer server;
    DataProvider& dataProvider;
    TaskHandle_t serverTaskHandle;
    String allowedIP;
    unsigned long lastFailedAttempt;
    int failedAttempts;
    
    void setupOTA();
    void setupRoutes();
    static void serverTask(void* parameter);
};

#endif 