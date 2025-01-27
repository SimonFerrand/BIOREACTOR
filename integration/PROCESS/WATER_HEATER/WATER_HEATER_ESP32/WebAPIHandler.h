#ifndef WEB_API_HANDLER_H
#define WEB_API_HANDLER_H

class AsyncWebServerRequest; // à garder?
#include <ESPAsyncWebServer.h>
#include "StateMachine.h"
#include "DataManager.h"

class WebAPIHandler {
public:
    WebAPIHandler(StateMachine& stateMachine);
    void handleGETRequest(AsyncWebServerRequest *request);
    void handlePOSTRequest(AsyncWebServerRequest *request);

private:
    StateMachine& _stateMachine;
    void handleDataRequest(AsyncWebServerRequest *request);
    void handleStatusRequest(AsyncWebServerRequest *request);
    void handleSystemInfoRequest(AsyncWebServerRequest *request);
    void sendJsonResponse(AsyncWebServerRequest *request, const String& data);
    
};

#endif