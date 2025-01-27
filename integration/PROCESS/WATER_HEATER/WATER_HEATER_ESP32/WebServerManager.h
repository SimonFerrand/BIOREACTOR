// WebServerManager.h
#ifndef WEBSERVER_MANAGER_H
#define WEBSERVER_MANAGER_H

#include <WebServer.h>
#include <ElegantOTA.h>
#include <Arduino.h>
#include "WebAPIHandler.h"
#include "config.h"
#include "Logger.h"
#include "StateMachine.h"
#include "WebPageBuilder.h"

class WebServerManager {
public:
    WebServerManager(WebAPIHandler& apiHandler, StateMachine& stateMachine);
    void begin();
    void handle();  // Called from task
    void stop();

private:
    WebServer server;
    WebAPIHandler& _apiHandler;
    StateMachine& _stateMachine;
    TaskHandle_t serverTaskHandle;

    String allowedIP;
    
    unsigned long lastFailedAttempt;
    int failedAttempts;
    
    void setupOTA();
    void setupRoutes();
    static void serverTask(void* parameter);

    static unsigned long lastRequest;
    static int requestCount;
    bool checkRateLimit();
};

#endif 