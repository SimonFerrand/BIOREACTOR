#include "StateMachine.h" // à garder pour éviter dépendance circulaire?
#include "WebAPIHandler.h"

WebAPIHandler::WebAPIHandler(StateMachine& stateMachine)
    : _stateMachine(stateMachine)
{
}

void WebAPIHandler::handleGETRequest(AsyncWebServerRequest *request) {
    String path = request->url();
    
    if (path == "/api/data") {
        handleDataRequest(request);
    }
    else if (path == "/api/status") {
        handleStatusRequest(request);
    }
    else if (path == "/api/system") {
        handleSystemInfoRequest(request);
    }
    else {
        request->send(404, "text/plain", "Not found");
    }
}

void WebAPIHandler::handlePOSTRequest(AsyncWebServerRequest *request) {
    request->send(405, "text/plain", "Method not allowed");
}

void WebAPIHandler::handleDataRequest(AsyncWebServerRequest *request) {
    SensorData data = DataManager::collectSensorData();
    String jsonData = DataManager::collectAllData(_stateMachine);
    sendJsonResponse(request, jsonData);
}

void WebAPIHandler::handleStatusRequest(AsyncWebServerRequest *request) {
    String status = DataManager::collectDeviceInfo();
    sendJsonResponse(request, status);
}

void WebAPIHandler::handleSystemInfoRequest(AsyncWebServerRequest *request) {
    String info = DataManager::collectDeviceInfo();
    sendJsonResponse(request, info);
}

void WebAPIHandler::sendJsonResponse(AsyncWebServerRequest *request, const String& data) {
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", data);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}