/*
 * ESP32 Bioreactor Control System
 * ------------------------------
 * Système de contrôle avancé pour bioréacteur basé sur ESP32
 * 
 * Composants Principaux :
 * ----------------------
 * - MQTTClient : Gestion des communications MQTT
 *   • Publication des données des capteurs
 *   • Réception des commandes
 *   • Heartbeat et monitoring de connexion
 * 
 * - WebServerManager : Interface web et mises à jour OTA
 *   • Dashboard de monitoring en temps réel
 *   • API REST pour l'accès aux données
 *   • Système de mise à jour OTA sécurisé
 * 
 * - SystemMonitor : Surveillance des ressources système
 *   • Monitoring mémoire et CPU
 *   • Surveillance de la qualité WiFi
 *   • Alertes et notifications
 * 
 * - DataProvider : Abstraction pour l'accès aux données des capteurs
 *   • Interface unifiée pour tous les capteurs
 *   • Mise en cache et agrégation des données
 *   • Validation et filtrage
 * 
 * - TaskManager : Gestion des tâches FreeRTOS
 *   • Distribution des tâches sur les cores
 *   • Gestion des priorités
 *   • Surveillance des ressources
 * 
 * - Logger : Système de logging unifié
 *   • Niveaux de log configurables
 *   • Horodatage et formatage
 *   • Support monitoring mémoire
 *
 * Configuration Matérielle :
 * ------------------------
 * - ESP32-S3 (dual core, 240MHz)
 * - Capteurs de température DS18B20
 * - Module WiFi intégré
 * - Flash 4MB minimum
 * 
 * How to perform an OTA update:
 * ----------------------------
 * 1. Build the binary:
 *    - In Arduino IDE: Sketch -> Export Compiled Binary
 *    - This creates a .bin file in your project folder
 *    - Use the main .bin file (typically named main.ino.bin, ~900KB)
 *    - DO NOT use .bootloader.bin, .merged.bin, or .partitions.bin
 * 
 * 2. Access the OTA interface:
 *    - Connect to the same network as your ESP32
 *    - Note the IP address shown in Serial Monitor
 *    - Open browser: http://[ESP_IP]/update. ex: http://192.168.1.27/update
 *    - Default credentials: admin/admin
 * 
 * 3. Upload the new firmware:
 *    - Select "Firmware" as OTA Mode
 *    - Click "Select File" and choose the .bin file
 *    - Click "Update" and wait for completion
 * 
 * Security Features:
 * ----------------
 * - IP-based access control for OTA
 * - Authentication required for web interface
 * - MQTT authentication
 * - Update attempt limiting
 * - Secure WiFi connection
 *
 * Software Setup:
 * -------------
 * 1. Install the ESP32 Board in Arduino IDE:
 *    - Open Arduino IDE
 *    - Go to File > Preferences
 *    - Add to Additional Board Manager URLs:
 *      https://dl.espressif.com/dl/package_esp32_index.json
 *      https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *    - Go to Tools > Board > Boards Manager
 *    - Install esp32 by Espressif Systems
 *    - Select ESP32S3 Dev Module
 * 
 * 2. Required Libraries:
 *    - WiFi.h : Connectivity
 *    - WebServer.h : Web interface
 *    - ArduinoJson : JSON parsing
 *    - AsyncMqttClient : MQTT communication
 *    - ElegantOTA : OTA updates
 *    - OneWire : Temperature sensors
 *    - FreeRTOS : Task management
 *    - EEPROM : Configuration storage
 * 
 * Configuration Options:
 * --------------------
 * - See config.h for all configurable parameters including:
 *   • WiFi credentials
 *   • MQTT broker settings
 *   • OTA security settings
 *   • Task priorities
 *   • Timing intervals
 *   • Debug levels
 *
 * Memory Management:
 * ----------------
 * - Recommended partition scheme:
 *   "Default 4MB with ffat (1.2MB APP/1.5MB FATFS)"
 * - Minimum heap required: 32KB
 * - Stack size per task: 4-8KB
 *
 * Debugging:
 * ---------
 * - Serial output at 115200 baud
 * - Web-based debug interface
 * - MQTT debug topics
 * - System metrics available via API
 */

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
#include "SensorController.h"
#include "DS18B20TemperatureSensor.h"


// Instances globales
Timezone myTZ;
MQTTClient mqttClient;
BioreactorDataProvider dataProvider;
WebServerManager* webServer = nullptr;
WebServer otaServer(81);  // Serveur OTA sur port 81
DS18B20TemperatureSensor waterTempSensor(15, "waterTempSensor");  // Water temperature sensor

// Tâche d'envoi des données
void dataSenderTask(void* parameter) {
    const TickType_t xFrequency = pdMS_TO_TICKS(15000); // 15 secondes
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (true) {
        if (mqttClient.isConnected()) {
            SensorData data = dataProvider.getLatestSensorData();
            
            // log des données de capteurs 
            Logger::log(Logger::LogLevel::INFO, "Water temperature: " + String(data.waterTemp) + "°C");
            
            
            if (mqttClient.publishSensorData(data)) {
                Logger::log(Logger::LogLevel::INFO, "Sensor data sent successfully");
            } else {
                Logger::log(Logger::LogLevel::ERROR, "Failed to send sensor data");
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
        Logger::log(Logger::LogLevel::ERROR, "Invalid command format");
        return;
    }
    
    if (command == "setParams") {
        Logger::log(Logger::LogLevel::INFO, "Processing setParams command");
    } else if (command == "getData") {
        SensorData data = dataProvider.getLatestSensorData();
        mqttClient.publishSensorData(data);
    } else {
        Logger::log(Logger::LogLevel::WARNING, "Unknown command: " + command);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Initialise sensors
    SensorController::initialize(waterTempSensor);
    SensorController::beginAll();

    Logger::log(Logger::LogLevel::INFO, "Starting ESP32 Bioreactor Control");
 
    // Initialiser le WiFi
    WiFiManager::initialize();
    WiFiManager::connect();
    
    // Attendre la connexion WiFi avec un timeout de 30 secondes
    int attempts = 0;
    while (!WiFiManager::isConnected() && attempts < 60) {
        delay(500);
        Logger::log(Logger::LogLevel::INFO, "Waiting for WiFi connection...");
        attempts++;
    }
    
    if (!WiFiManager::isConnected()) {
        Logger::log(Logger::LogLevel::ERROR, "Failed to connect to WiFi. Restarting...");
        ESP.restart();
        return;
    }

    // Setup time APRÈS la connexion WiFi
    Logger::log(Logger::LogLevel::INFO, "Setting up time synchronization...");
    waitForSync();
    myTZ.setLocation(F("Europe/Paris"));
    Logger::log(Logger::LogLevel::INFO, "Time synchronized: " + myTZ.dateTime());
    
    // Initialiser le monitoring système
    SystemMonitor::initialize();
    
    // Configuration des callbacks MQTT
    mqttClient.onConnectionEstablished([]() {
        Logger::log(Logger::LogLevel::INFO, "MQTT Connected - Starting tasks");
        
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
        Logger::log(Logger::LogLevel::ERROR, "MQTT Connection lost");
    });
    
    mqttClient.onMessageReceived([](const char* topic, const char* message) {
        Logger::log(Logger::LogLevel::INFO, "Message received on topic: " + String(topic));
        
        if (strcmp(topic, MQTT_TOPIC_COMMANDS) == 0) {
            processCommand(message);
        }
    });
    
    // Démarrer le client MQTT
    mqttClient.begin();
    
    // Créer et démarrer le serveur Web
    webServer = new WebServerManager(dataProvider);
    webServer->begin();
    Logger::log(Logger::LogLevel::INFO, "Web server started on port 80");
    
    // Démarrer le monitoring système
    SystemMonitor::startMonitoring();
    
    Logger::log(Logger::LogLevel::INFO, "Setup completed");

}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}