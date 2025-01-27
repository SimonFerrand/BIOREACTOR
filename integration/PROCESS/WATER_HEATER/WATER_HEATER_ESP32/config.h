// ===== config.h =====
#ifndef CONFIG_H
#define CONFIG_H

#include "private_config.h"

// Sensor Pin Configuration
/*
  // PT100 (SPI Pin Configuration for MAX31865)
#define MAX31865_CS_PIN    5     // Chip Select pour MAX31865 
#define MAX31865_MOSI_PIN  23    // SPI MOSI                  
#define MAX31865_MISO_PIN  19    // SPI MISO                   
#define MAX31865_CLK_PIN   18    // SPI CLK
*/
  // DS18B20
#define DS18B20_PIN 32

  // Pressure Pin Configuration
#define PRESSURE_SENSOR_PIN 35    // ADC0/GPIO35 pour le capteur de pression

// Actuator Pin Configuration
#define HEATING_PLATE_PIN  12     // Pin pour le contrôle de la plaque chauffante   // Heating plate (Relay: 12, Not PWM capable) - 24V
#define HEATING_PLATE_PWM_CAPABLE false   // Si la plaque supporte le PWM

// Network Ports
#define OTA_SERVER_PORT 81       // Port pour les mises à jour OTA

// ESP32 Core Settings
#define MQTT_CORE 0            // Core pour MQTT
#define SENSOR_CORE 1          // Core pour les capteurs

// OTA
static const char* ALLOWED_IP = "192.168.1.122";  // La seule adresse IP autorisé à téléversé sur l'ESP32
#define STACK_SIZE_WEBSERVER 8192  

// MQTT Configuration
#define MQTT_BROKER "192.168.1.25" // Adress du serveur
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "electric_water_heater"

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
#define STACK_SIZE_MONITOR 4096 

// Timing Configuration
#define MQTT_RECONNECT_INTERVAL 5000
#define MQTT_HEARTBEAT_INTERVAL 30000
#define SENSOR_READ_INTERVAL 5000
#define MONITOR_CHECK_INTERVAL 60000

// Task Intervals (in milliseconds)
// pdMS_TO_TICKS converts milliseconds to FreeRTOS ticks for precise timing control
#define TASK_INTERVAL_STATEMACHINE    1000  // State machine update interval
#define TASK_INTERVAL_SAFETY          1000  // Safety check interval
#define TASK_INTERVAL_DATASENDER      15000 // MQTT data sending interval
#define TASK_INTERVAL_COMMAND         100   // Command checking interval

// PID update
#define PID_UPDATE_TEMP 15000

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

// Temperature Safety Limits
#define MIN_WATER_TEMP 15.0f       // Minimum safe water temperature
#define MAX_WATER_TEMP 40.0f       // Maximum safe water temperature
#define CRITICAL_WATER_TEMP 45.0f  // Critical water temperature threshold

#define API_REQUEST_WINDOW 60000 // 1 minute
#define API_MAX_REQUESTS 10

#endif // CONFIG_H