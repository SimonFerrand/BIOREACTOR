// config.h
#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials
const char ssid[] = "oOo";
const char password[] = "wYnXLVYW42yknz7VG4Ab25E8";

// Authentication variable
const uint8_t sharedSecret[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

// MQTT Configuration
const char* MQTT_HOST = "192.168.1.25";
const uint16_t MQTT_PORT = 1883;
const char* MQTT_CLIENT_ID = "ESP32_Bioreactor";
const char* MQTT_COMMAND_TOPIC = "bioreactor/commands";
const char* MQTT_STATUS_TOPIC = "bioreactor/status";
const char* MQTT_SENSOR_TOPIC = "bioreactor/sensors";

// Debug configuration
#define ENABLE_DEBUG true
#define DEBUG_SERIAL if(ENABLE_DEBUG) Serial

#endif // CONFIG_H