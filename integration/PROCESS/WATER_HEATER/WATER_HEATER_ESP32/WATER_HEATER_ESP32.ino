/*
 * ESP32 Water Heater Control System
 * ------------------------------
 * Système de contrôle avancé pour bioréacteur basé sur ESP32
 * 
 * Composants Principaux :
 * ----------------------
 * - PIDManager : Système de contrôle PID
 *   • Contrôle de température
 *   • Modes startup/maintain
 *   • Gestion de l'hystérésis
 *
 * - SafetySystem : Système de sécurité
 *   • Surveillance des limites critiques
 *   • Arrêt d'urgence automatique
 *   • Gestion des alarmes
 * 
 * - MQTTClient : Communication MQTT
 *   • Publication des données des capteurs
 *   • Réception des commandes
 *   • Heartbeat et monitoring
 * 
 * - WebServerManager : Interface web et mises à jour OTA
 *   • Dashboard de monitoring en temps réel
 *   • API REST pour l'accès aux données
 *   • Système de mise à jour OTA sécurisé
 * 
 * - DataManager : Gestion des données
 *   • Collection des données capteurs
 *   • Agrégation et formatage
 *   • Distribution aux composants
 *
 * - SystemMonitor : Surveillance des ressources système
 *   • Monitoring mémoire et CPU
 *   • Surveillance de la qualité WiFi
 *   • Alertes et notifications
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
 * - PT100 avec MAX31865
 * - Capteur de pression industriel (4-20mA)
 * - Plaque chauffante avec contrôle
 * - Flash 4MB minimum
 * 
 * Programmes inclus :
 * -----------------
 * - Programme de stérilisation sous pression
 * - Programme CIP (Clean In Place)
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
 *    - Select ESP32 Dev Module
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
#include <ezTime.h>
#include "config.h"
#include "Logger.h"
#include "WiFiManager.h"
#include "TaskManager.h"
#include "DataManager.h"
#include "SystemMonitor.h"
#include "MQTTClient.h"
#include "WebServerManager.h"
#include "SensorController.h"
#include "ActuatorController.h"
#include "PT100Sensor.h"
#include "DS18B20TemperatureSensor.h"
#include "PressureSensor.h"
#include "HeatingPlate.h"
#include "SafetySystem.h"
#include "WebAPIHandler.h"
#include "PressureSterilizationProgram.h"
#include "CIPProgram.h"
#include "CommandHandler.h"


// Instances globales
Timezone myTZ;
PIDManager pidManager;  
MQTTClient mqttClient;
StateMachine stateMachine(pidManager, mqttClient);
WebAPIHandler webAPI(stateMachine);
WebServerManager webServer(webAPI, stateMachine);
WebServer otaServer(OTA_SERVER_PORT);  
CommandHandler commandHandler(stateMachine, mqttClient);

// Sensor declarations
DS18B20TemperatureSensor waterTempSensor(DS18B20_PIN, "waterTempSensor");
//PT100Sensor waterTempSensor(MAX31865_CS_PIN, MAX31865_MOSI_PIN, MAX31865_MISO_PIN, MAX31865_CLK_PIN, "waterTempSensor2");  
PressureSensor pressureSensor(PRESSURE_SENSOR_PIN, "pressureSensor");  

// Actuator declarations
HeatingPlate heatingPlate(HEATING_PLATE_PIN, HEATING_PLATE_PWM_CAPABLE, "heatingPlate");            

// System components

// Safety System
SafetySystem safetySystem(stateMachine);

// Program decaration
PressureSterilizationProgram pressureSterilizationProgram(pidManager);
CIPProgram cipProgram(pidManager);

void setup() {
    Serial.begin(115200);
    delay(1000);

    //   Initialize sensors and actuators with verification
    bool sensorsOk = SensorController::initialize(waterTempSensor, pressureSensor);
    bool sensorsStarted = SensorController::beginAll();
    bool actuatorsOk = ActuatorController::initialize(heatingPlate);
    bool actuatorsStarted = ActuatorController::beginAll();

    //  Record global initialization status
    Logger::log(Logger::LogLevel::INFO, "System initialization status");

    String sensorStatus = String("- Sensors init: ") + String(sensorsOk) + 
                        String(", Started: ") + String(sensorsStarted);
    Logger::log(Logger::LogLevel::INFO, sensorStatus);

    String actuatorStatus = String("- Actuators init: ") + String(actuatorsOk) + 
                          String(", Started: ") + String(actuatorsStarted);
    Logger::log(Logger::LogLevel::INFO, actuatorStatus);

    // In downgraded mode if necessary
    if (!sensorsOk || !sensorsStarted || !actuatorsOk || !actuatorsStarted) {
        Logger::log(Logger::LogLevel::WARNING, 
            F("System starting in degraded mode (bug with sensor or actuator)"));
    }

    // Initialize programms 
    stateMachine.addProgram("PressureSterilization", &pressureSterilizationProgram);
    stateMachine.addProgram("CIP", &cipProgram);

    // Initialize PID
    pidManager.initialize(2.0, 5.0, 1.0);
    pidManager.setHysteresis(0.5);
    Logger::log(Logger::LogLevel::INFO, F("PID setup"));

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
    Logger::log(Logger::LogLevel::INFO, F("Setting up time synchronization..."));
    waitForSync();
    myTZ.setLocation(F("Europe/Paris"));
    Logger::log(Logger::LogLevel::INFO, "Time synchronized: " + myTZ.dateTime());

    // Initialize Task
    if (!safetySystem.begin() ||
        !stateMachine.begin() ||
        !mqttClient.begin() ||
        !commandHandler.begin()) {
        Logger::log(Logger::LogLevel::ERROR, "Task initialization failed");
        return;
    }

    // Initialiser le serveur
    webServer.begin();

    // Initialiser le monitoring système
    SystemMonitor::initialize();
    
    mqttClient.onConnectionLost([]() {
        Logger::log(Logger::LogLevel::ERROR, F("MQTT Connection lost"));
    });
    
    mqttClient.onMessageReceived([](const char* topic, const char* message) {
        Logger::log(Logger::LogLevel::INFO, "Message received on topic: " + String(topic));
        
        if (strcmp(topic, MQTT_TOPIC_COMMANDS) == 0) {
            commandHandler.handleJsonCommand(String(message));
        }
    });
    
    // Démarrer le client MQTT
    mqttClient.setStateMachine(&stateMachine);
    mqttClient.begin();
    
    // Créer et démarrer le serveur Web
    Logger::log(Logger::LogLevel::INFO, F("Web server started on port 80"));
    
    // Démarrer le monitoring système
    SystemMonitor::startMonitoring();
    
    Logger::log(Logger::LogLevel::INFO, F("Setup completed"));

}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}