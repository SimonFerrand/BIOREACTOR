// ===== config.h =====
#ifndef CONFIG_H
#define CONFIG_H

#include "private_config.h"

// ESP32 Core Settings
#define MQTT_CORE 0            // Core pour MQTT
#define SENSOR_CORE 1          // Core pour les capteurs

// OTA
static const char* ALLOWED_IP = "192.168.1.122";  // La seule adresse IP autorisé à téléversé sur l'ESP32
#define STACK_SIZE_WEBSERVER 8192  // Augmenter la taille de 4096 à 8192

// MQTT Configuration
#define MQTT_BROKER "192.168.1.25" // Adress du serveur
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "water_bath"

// MQTT Topics
#define MQTT_TOPIC_STATUS "water_bath/status"
#define MQTT_TOPIC_SENSORS "water_bath/sensors"
#define MQTT_TOPIC_COMMANDS "water_bath/commands"

// OTA Settings 
#define OTA_USERNAME "admin"
#define OTA_PASSWORD "admin"
#define OTA_MAX_ATTEMPTS 3
#define OTA_BLOCK_TIME 300000  // 5 minutes

// Task Configuration
#define TASK_PRIORITY_HIGH 5
#define TASK_PRIORITY_MEDIUM 3
#define TASK_PRIORITY_LOW 1

#define STACK_SIZE_MQTT 4096
#define STACK_SIZE_SENSORS 4096
#define STACK_SIZE_MONITOR 2048

// Timing Configuration
#define MQTT_RECONNECT_INTERVAL 5000
#define MQTT_HEARTBEAT_INTERVAL 30000
#define SENSOR_READ_INTERVAL 5000
#define MONITOR_CHECK_INTERVAL 10000

// Buffer Sizes
#define JSON_BUFFER_SIZE 1024  
#define MQTT_QUEUE_SIZE 20

//
#define MQTT_MAX_RETRIES 5

// WiFi Power Configuration
    // Available power levels (from highest to lowest) :
          // WIFI_POWER_19_5dBm : 19.5 dBm (~89 mW)  - Maximum power, longest range
          // WIFI_POWER_18_5dBm : 18.5 dBm (~71 mW)
          // WIFI_POWER_17dBm   : 17.0 dBm (~50 mW)
          // WIFI_POWER_15dBm   : 15.0 dBm (~32 mW)  - Good for indoor 15-25m
          // WIFI_POWER_13dBm   : 13.0 dBm (~20 mW)  - Good for indoor 10-20m
          // WIFI_POWER_11dBm   : 11.0 dBm (~13 mW)  - Good for indoor 5-15m
          // WIFI_POWER_8_5dBm  : 8.5 dBm  (~7 mW)
          // WIFI_POWER_7dBm    : 7.0 dBm  (~5 mW)   - Power saving, short range
          // WIFI_POWER_5dBm    : 5.0 dBm  (~3 mW)
          // WIFI_POWER_2dBm    : 2.0 dBm  (~1.6 mW)
          // WIFI_POWER_MINUS_1dBm : -1.0 dBm (~0.8 mW) - Minimum power
#define WIFI_POWER_LEVEL WIFI_POWER_15dBm  // Choisir parmi les options ci-dessus

// Debugging options
#define DEBUG_LEVEL 3  // 0=OFF, 1=ERROR, 2=INFO, 3=DEBUG

#endif // CONFIG_H