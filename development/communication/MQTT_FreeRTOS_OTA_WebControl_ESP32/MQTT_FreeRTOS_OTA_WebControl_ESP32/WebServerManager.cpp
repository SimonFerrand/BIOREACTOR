// WebServerManager.cpp
#include "WebServerManager.h"

WebServerManager::WebServerManager(DataProvider& provider) 
    : server(80)
    , dataProvider(provider)
    , serverTaskHandle(nullptr)
    , failedAttempts(0)
    , lastFailedAttempt(0)
    , allowedIP(ALLOWED_IP) {
}

void WebServerManager::begin() {
    setupRoutes();
    setupOTA();
    
    xTaskCreatePinnedToCore(
        serverTask,
        "WebServer",
        4096,
        this,
        1,
        &serverTaskHandle,
        0
    );
    
    server.begin();
    Logger::log(Logger::INFO, "Web server started at http://" + WiFi.localIP().toString());
}

void WebServerManager::setupOTA() {
    ElegantOTA.begin(&server, OTA_USERNAME, OTA_PASSWORD);
    ElegantOTA.setAutoReboot(false);

    ElegantOTA.onStart([this]() {
        String clientIP = server.client().remoteIP().toString();
        Logger::log(Logger::INFO, "OTA Update Started from " + clientIP);

        if (clientIP != allowedIP) {
            Logger::log(Logger::ERROR, "OTA Access Denied - Unauthorized IP");
            return;
        }

        if (failedAttempts >= OTA_MAX_ATTEMPTS) {
            unsigned long timeLeft = (OTA_BLOCK_TIME - (millis() - lastFailedAttempt)) / 1000;
            if (timeLeft > 0) {
                Logger::log(Logger::ERROR, "Too many attempts. Wait " + String(timeLeft) + " seconds");
                return;
            }
            failedAttempts = 0;
        }
    });

    ElegantOTA.onProgress([](size_t progress, size_t total) {
        static size_t lastProgress = 0;
        static unsigned long lastDisplayTime = 0;

        if (millis() - lastDisplayTime < 1000) return;
        lastDisplayTime = millis();

        float speed = (progress - lastProgress) / 1024.0;
        lastProgress = progress;

        Logger::log(Logger::INFO, 
            "Progress: " + String((progress * 100) / total) + "% (" + 
            String(speed, 1) + " KB/s)"
        );
    });

    ElegantOTA.onEnd([this](bool success) {
        if (success) {
            Logger::log(Logger::INFO, "OTA Update Successful - Restarting...");
            delay(500);
            ESP.restart();
        } else {
            failedAttempts++;
            lastFailedAttempt = millis();
            Logger::log(Logger::ERROR, 
                "Update Failed (Attempt " + String(failedAttempts) + "/" + 
                String(OTA_MAX_ATTEMPTS) + ")"
            );
        }
    });
}

void WebServerManager::setupRoutes() {
    server.on("/", HTTP_GET, [this]() {
        String html = WebPageBuilder::buildIndexPage(dataProvider.getDeviceInfo());
        server.send(200, "text/html", html);
    });
    
    server.on("/data", HTTP_GET, [this]() {
        SensorData data = dataProvider.getLatestSensorData();
        String html = WebPageBuilder::buildDataPage(data);
        server.send(200, "text/html", html);
    });
    
    server.on("/api/data", HTTP_GET, [this]() {
        SensorData data = dataProvider.getLatestSensorData();
        String json = APIHandler::serializeData(data);
        server.send(200, "application/json", json);
    });
}

void WebServerManager::handle() {
    server.handleClient();
    ElegantOTA.loop();
}

void WebServerManager::serverTask(void* parameter) {
    WebServerManager* manager = static_cast<WebServerManager*>(parameter);
    const TickType_t xDelay = pdMS_TO_TICKS(10);
    
    while (true) {
        manager->handle();
        vTaskDelay(xDelay);
    }
}

void WebServerManager::stop() {
    if (serverTaskHandle) {
        vTaskDelete(serverTaskHandle);
        serverTaskHandle = nullptr;
    }
    server.stop();
}
