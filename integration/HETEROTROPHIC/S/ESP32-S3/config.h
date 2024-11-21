// config.h
#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials
static const char* WIFI_SSID = "oOo";
static const char* WIFI_PASSWORD = "wYnXLVYW42yknz7VG4Ab25E8";

// OTA Settings
#define OTA_USERNAME "admin"
#define OTA_PASSWORD "admin"
#define OTA_MAX_ATTEMPTS 3
#define OTA_BLOCK_TIME 300000  // 5 minutes
static const char* ALLOWED_IP = "192.168.1.122";

#endif // CONFIG_H