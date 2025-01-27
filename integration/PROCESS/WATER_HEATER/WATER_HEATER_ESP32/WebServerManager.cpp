// WebServerManager.cpp
#include "WebServerManager.h"

// Variables statiques
unsigned long WebServerManager::lastRequest = 0;
int WebServerManager::requestCount = 0;

WebServerManager::WebServerManager(WebAPIHandler& apiHandler, StateMachine& stateMachine)
    : server(80)
    , _apiHandler(apiHandler)
    , _stateMachine(stateMachine)
    , serverTaskHandle(nullptr)
    , allowedIP(ALLOWED_IP)
    , lastFailedAttempt(0)
    , failedAttempts(0)
{
}

void WebServerManager::begin() {
    setupRoutes();
    setupOTA();
    
    xTaskCreatePinnedToCore(
        serverTask,
        "WebServer",
        STACK_SIZE_WEBSERVER, 
        this,
        1,
        &serverTaskHandle,
        0
    );
    
    server.begin();
    Logger::log(Logger::LogLevel::INFO, "Web server started at http://" + WiFi.localIP().toString());
}

/*
void WebServerManager::setupOTA() {
    Logger::log(Logger::LogLevel::INFO, "Setting up OTA...");
    ElegantOTA.begin(&server, OTA_USERNAME, OTA_PASSWORD);
    ElegantOTA.setAutoReboot(false);

    ElegantOTA.onStart([this]() {
        String clientIP = server.client().remoteIP().toString();
        Logger::log(Logger::LogLevel::INFO, "OTA Update Started from " + clientIP);
        Logger::log(Logger::LogLevel::INFO, "Comparing with allowed IP: " + String(ALLOWED_IP));
        
        if (clientIP != allowedIP) {
            Logger::log(Logger::LogLevel::ERROR, "OTA Access Denied - Unauthorized IP");
            return;
        }

        if (failedAttempts >= OTA_MAX_ATTEMPTS) {
            unsigned long timeLeft = (OTA_BLOCK_TIME - (millis() - lastFailedAttempt)) / 1000;
            if (timeLeft > 0) {
                Logger::log(Logger::LogLevel::LogLevel::ERROR, "Too many attempts. Wait " + String(timeLeft) + " seconds");
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

        Logger::log(Logger::LogLevel::LogLevel::INFO, 
            "Progress: " + String((progress * 100) / total) + "% (" + 
            String(speed, 1) + " KB/s)"
        );
    });

    ElegantOTA.onEnd([this](bool success) {
        if (success) {
            Logger::log(Logger::LogLevel::LogLevel::INFO, "OTA Update Successful - Restarting...");
            delay(500);
            ESP.restart();
        } else {
            failedAttempts++;
            lastFailedAttempt = millis();
            Logger::log(Logger::LogLevel::LogLevel::ERROR, 
                "Update Failed (Attempt " + String(failedAttempts) + "/" + 
                String(OTA_MAX_ATTEMPTS) + ")"
            );
        }
    });
}

*/

void WebServerManager::setupOTA() {
    ElegantOTA.begin(&server, OTA_USERNAME, OTA_PASSWORD);
    ElegantOTA.setAutoReboot(false);

    ElegantOTA.onStart([this]() -> bool {
        String clientIP = server.client().remoteIP().toString();
        Logger::log(Logger::LogLevel::INFO, "🚀 OTA Update Started");
        Logger::log(Logger::LogLevel::INFO, "📡 Client: " + clientIP);
        Logger::log(Logger::LogLevel::INFO, "Allowed IP: " + String(allowedIP));

        if (clientIP != this->allowedIP) { 
            Logger::log(Logger::LogLevel::ERROR, "⛔ Access Denied - Unauthorized IP");
            return false;
        }

        if (failedAttempts >= OTA_MAX_ATTEMPTS) {
            unsigned long timeLeft = (OTA_BLOCK_TIME - (millis() - lastFailedAttempt)) / 1000;
            if (timeLeft > 0) {
                Logger::log(Logger::LogLevel::ERROR, "⏳ Too many attempts. Wait " + String(timeLeft) + " seconds");
                return false;
            }
            failedAttempts = 0;
        }

        Logger::log(Logger::LogLevel::INFO, "✅ Update Authorized");
        return true;
    });

    ElegantOTA.onProgress([](size_t progress, size_t total) {
        static size_t lastProgress = 0;
        static unsigned long lastDisplayTime = 0;
        
        // Éviter les divisions par zéro
        if (total == 0) {
            Logger::log(Logger::LogLevel::ERROR, "Invalid total size");
            return;
        }

        // Limiter la fréquence des mises à jour
        unsigned long currentTime = millis();
        if (currentTime - lastDisplayTime < 1000) {
            return;
        }
        lastDisplayTime = currentTime;

        // Calculer les valeurs avec vérification
        if (progress <= total) {  
            int percentage = (progress * 100) / total;
            float speed = 0;
            
            if (progress > lastProgress) {
                speed = (progress - lastProgress) / 1024.0;  // KB/s
            }
            lastProgress = progress;

            Logger::log(Logger::LogLevel::INFO, "📊 Progress: " + String(percentage) + "% (" + 
                String(speed, 1) + " KB/s)");
        }
    });

    ElegantOTA.onEnd([this](bool success) {
        if (success) {
            Logger::log(Logger::LogLevel::INFO, "✅ Update Successfully Completed!");
            Logger::log(Logger::LogLevel::INFO, "🔄 Restarting Device...");
            delay(1000);  
            ESP.restart();
        } else {
            failedAttempts++;
            lastFailedAttempt = millis();
            Logger::log(Logger::LogLevel::ERROR, "❌ Update Failed (Attempt " + 
                String(failedAttempts) + "/" + String(OTA_MAX_ATTEMPTS) + ")");
        }
    });

    Logger::log(Logger::LogLevel::INFO, "OTA setup completed");
}

bool WebServerManager::checkRateLimit() {
    if (millis() - lastRequest > API_REQUEST_WINDOW) {
        requestCount = 0;
        lastRequest = millis();
    }
    if (requestCount >= API_MAX_REQUESTS) return false;
    requestCount++;
    return true;
}

// setupRoutes complet
void WebServerManager::setupRoutes() {
    Logger::log(Logger::LogLevel::INFO, "Setting up web routes...");
    server.on("/", HTTP_GET, [this]() {
        if (!checkRateLimit()) {
            server.send(429, "text/plain", "Too many requests");
            return;
        }
        String html = WebPageBuilder::buildIndexPage(DataManager::collectDeviceInfo());
        server.send(200, "text/html", html);
    });
    
    server.on("/data", HTTP_GET, [this]() {
        if (!checkRateLimit()) {
            server.send(429, "text/plain", "Too many requests");
            return;
        }
        SensorData data = DataManager::collectSensorData();
        String html = WebPageBuilder::buildDataPage(data);
        server.send(200, "text/html", html);
    });
    
    server.on("/api/data", HTTP_GET, [this]() {
        if (!checkRateLimit()) {
            server.send(429, "text/plain", "Too many requests");
            return;
        }
        String json = DataManager::collectAllData(_stateMachine);
        server.send(200, "application/json", json);
    });

    server.on("/api/system", HTTP_GET, [this]() {
        if (!checkRateLimit()) {
            server.send(429, "text/plain", "Too many requests");
            return;
        }
        String info = DataManager::collectSystemMetrics();
        server.send(200, "application/json", info);
    });

    server.on("/api/status", HTTP_GET, [this]() {
        if (!checkRateLimit()) {
            server.send(429, "text/plain", "Too many requests");
            return;
        }
        String status = DataManager::collectDeviceInfo();
        server.send(200, "application/json", status);
    });

    server.on("/cip", HTTP_GET, [this]() {
        if (server.hasArg("temp") && server.hasArg("duration")) {
            String temp = server.arg("temp");
            String duration = server.arg("duration");
            String command = "cip " + temp + " " + duration;
            _stateMachine.startProgram("CIP", command);
            server.send(200, "text/plain", "CIP started: " + temp + "°C for " + duration + " minutes");
        } else {
            server.send(400, "text/plain", "Use: /cip?temp=30&duration=30");
        }
    });

    server.on("/stop", HTTP_GET, [this]() {
      _stateMachine.stopAllPrograms();
      server.send(200, "text/plain", "All programs stopped");
    });

    server.on("/program", HTTP_GET, [this]() {
      String html = WebPageBuilder::buildProgramPage();
      server.send(200, "text/html", html);
    });

}

void WebServerManager::handle() {
    if (server.client()) {
        String clientIP = server.client().remoteIP().toString();
        Logger::log(Logger::LogLevel::DEBUG, "Request from: " + clientIP);
    }
    
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
