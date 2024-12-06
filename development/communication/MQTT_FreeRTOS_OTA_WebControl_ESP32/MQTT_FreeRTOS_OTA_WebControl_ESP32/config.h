// ===== config.h =====
#ifndef CONFIG_H
#define CONFIG_H

#include "private_config.h"

// ESP32 Core Settings
#define MQTT_CORE 0            // Core pour MQTT
#define SENSOR_CORE 1          // Core pour les capteurs

// MQTT Configuration
#define MQTT_BROKER "192.168.1.25"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "esp32_bioreactor"

// MQTT Topics
#define MQTT_TOPIC_STATUS "bioreactor/status"
#define MQTT_TOPIC_SENSORS "bioreactor/sensors"
#define MQTT_TOPIC_COMMANDS "bioreactor/commands"

// OTA Settings 
#define OTA_USERNAME "admin"
#define OTA_PASSWORD "admin"
#define OTA_MAX_ATTEMPTS 3
#define OTA_BLOCK_TIME 300000  // 5 minutes
static const char* ALLOWED_IP = "192.168.1.122";  // Votre IP

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

// ESP32 Specific Settings
#define WIFI_POWER 20          // WiFi TX power (2-20 dBm)
#define WIFI_PROTOCOL WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N

// Debugging options
#define DEBUG_LEVEL 2  // 0=OFF, 1=ERROR, 2=INFO, 3=DEBUG

#endif // CONFIG_H