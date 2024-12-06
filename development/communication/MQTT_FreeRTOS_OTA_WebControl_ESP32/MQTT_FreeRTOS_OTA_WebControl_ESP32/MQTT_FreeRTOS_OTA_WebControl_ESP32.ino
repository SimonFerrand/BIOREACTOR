
#include <Arduino.h>
#include <WebServer.h>
#include "config.h"
#include "Logger.h"
#include "WiFiManager.h"
#include "TaskManager.h"
#include "DataProvider.h"
#include "SystemMonitor.h"
#include "MQTTClient.h"
#include "WebServerManager.h"

// Instances globales
MQTTClient mqttClient;
BioreactorDataProvider dataProvider;
WebServerManager* webServer = nullptr;
WebServer otaServer(81);  // Serveur OTA sur port 81

// Tâche d'envoi des données
void dataSenderTask(void* parameter) {
    const TickType_t xFrequency = pdMS_TO_TICKS(5000); // 5 secondes
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (true) {
        if (mqttClient.isConnected()) {
            SensorData data = dataProvider.getLatestSensorData();
            
            if (mqttClient.publishSensorData(data)) {
                Logger::log(Logger::INFO, "Sensor data sent successfully");
            } else {
                Logger::log(Logger::ERROR, "Failed to send sensor data");
            }
        }
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// Traitement des commandes reçues
void processCommand(const char* message) {
    String command;
    JsonDocument params;
    
    if (!MessageFormatter::parseCommand(String(message), command, params)) {
        Logger::log(Logger::ERROR, "Invalid command format");
        return;
    }
    
    if (command == "setParams") {
        Logger::log(Logger::INFO, "Processing setParams command");
    } else if (command == "getData") {
        SensorData data = dataProvider.getLatestSensorData();
        mqttClient.publishSensorData(data);
    } else {
        Logger::log(Logger::WARNING, "Unknown command: " + command);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Logger::log(Logger::INFO, "Starting ESP32 Bioreactor Control");
    
    // Initialiser le WiFi
    WiFiManager::initialize();
    WiFiManager::connect();
    
    // Attendre la connexion WiFi avec un timeout de 30 secondes
    int attempts = 0;
    while (!WiFiManager::isConnected() && attempts < 60) {
        delay(500);
        Logger::log(Logger::INFO, "Waiting for WiFi connection...");
        attempts++;
    }
    
    if (!WiFiManager::isConnected()) {
        Logger::log(Logger::ERROR, "Failed to connect to WiFi. Restarting...");
        ESP.restart();
        return;
    }
    
    // Initialiser le monitoring système
    SystemMonitor::initialize();
    
    // Configuration des callbacks MQTT
    mqttClient.onConnectionEstablished([]() {
        Logger::log(Logger::INFO, "MQTT Connected - Starting tasks");
        
        TaskManager::createTask(
            dataSenderTask,
            "DataSender",
            STACK_SIZE_SENSORS,
            nullptr,
            TASK_PRIORITY_LOW,
            SENSOR_CORE
        );
    });
    
    mqttClient.onConnectionLost([]() {
        Logger::log(Logger::ERROR, "MQTT Connection lost");
    });
    
    mqttClient.onMessageReceived([](const char* topic, const char* message) {
        Logger::log(Logger::INFO, "Message received on topic: " + String(topic));
        
        if (strcmp(topic, MQTT_TOPIC_COMMANDS) == 0) {
            processCommand(message);
        }
    });
    
    // Démarrer le client MQTT
    mqttClient.begin();
    
    // Créer et démarrer le serveur Web
    webServer = new WebServerManager(dataProvider);
    webServer->begin();
    Logger::log(Logger::INFO, "Web server started on port 80");
    
    // Démarrer le monitoring système
    SystemMonitor::startMonitoring();
    
    Logger::log(Logger::INFO, "Setup completed");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}