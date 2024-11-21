// OTAManager.cpp
#include "OTAManager.h"

OTAManager::OTAManager(WebServer* server) {
    this->server = server;
    this->failedAttempts = 0;
    this->lastFailedAttempt = 0;
    this->allowedIP = String(ALLOWED_IP);  
}

void OTAManager::setupCallbacks() {
    // Initialize progress monitoring variables
    static size_t lastProgress = 0;
    static unsigned long lastDisplayTime = 0;
    
    ElegantOTA.onStart([this]() {
        String clientIP = server->client().remoteIP().toString();
        Serial.println("\n=====================================");
        Serial.println("🚀 OTA Update Started");
        Serial.printf("📡 Client: %s\n", clientIP.c_str());
        
        if (clientIP != allowedIP) {
            Serial.println("⛔ Access Denied - Unauthorized IP");
            return;
        }
        
        if (failedAttempts >= OTA_MAX_ATTEMPTS) {
            unsigned long timeLeft = (OTA_BLOCK_TIME - (millis() - lastFailedAttempt)) / 1000;
            if (timeLeft > 0) {
                Serial.printf("⏳ Too many attempts. Wait %lu seconds\n", timeLeft);
                return;
            }
            failedAttempts = 0;
        }
        
        lastProgress = 0;  // Tracking reset
        Serial.println("✅ Update Authorized\n");
    });
    
    ElegantOTA.onProgress([](size_t progress, size_t total) {
        // Limit updates to once per second
        if (millis() - lastDisplayTime < 1000) return;
        lastDisplayTime = millis();

        //  Display raw information for debugging
        float progressKB = progress / 1024.0;
        float totalKB = total / 1024.0;
        
        // Calculate transfer speed
        float speed = (progress - lastProgress) / 1024.0;  // KB/s
        lastProgress = progress;

        Serial.printf("📊 Transfer Status:\n");
        Serial.printf("   Received: %.1f KB\n", progressKB);
        Serial.printf("   Total: %.1f KB\n", totalKB);
        Serial.printf("   Speed: %.1f KB/s\n", speed);
        Serial.println("-------------------------------------");
    });
    
    ElegantOTA.onEnd([this](bool success) {
        Serial.println("\n=====================================");
        if (success) {
            Serial.println("✅ Update Successfully Completed!");
            Serial.println("🔄 Restarting Device...");
            delay(500);  
            ESP.restart();
        } else {
            failedAttempts++;
            lastFailedAttempt = millis();
            Serial.printf("❌ Update Failed (Attempt %d/%d)\n", failedAttempts, OTA_MAX_ATTEMPTS);
        }
        Serial.println("=====================================\n");
    });
}

void OTAManager::begin() {
    ElegantOTA.begin(server, OTA_USERNAME, OTA_PASSWORD);
    ElegantOTA.setAutoReboot(false);
    setupCallbacks();
    Serial.println("OTA access available on http://" + WiFi.localIP().toString() + "/update");
}

void OTAManager::loop() {
    server->handleClient();
    ElegantOTA.loop();
}