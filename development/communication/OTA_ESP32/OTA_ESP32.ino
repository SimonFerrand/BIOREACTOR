
// main.cpp
/*
 * ESP32-S3 OTA Update System
 * ------------------------
 * This code implements secure OTA (Over-The-Air) updates for ESP32-S3 using ElegantOTA.
 * 
 * How to perform an OTA update:
 * ----------------------------
 * 1. Build the binary:
 *    - In Arduino IDE: Sketch -> Export Compiled Binary
 *    - This creates a .bin file in your project folder
 *    - Use the main .bin file (typically named ESP32-S3.ino.bin, ~900KB)
 *    - DO NOT use .bootloader.bin, .merged.bin, or .partitions.bin
 * 
 * 2. Access the OTA interface:
 *    - Connect to the same network as your ESP32
 *    - Note the IP address shown in Serial Monitor
 *    - Open browser: http://[ESP_IP]/update. ex: http://192.168.20.20/update
 *    - Default credentials: admin/admin
 * 
 * 3. Upload the new firmware:
 *    - Select "Firmware" as OTA Mode
 *    - Click "Select File" and choose the .bin file
 *    - Click "Update" and wait for completion
 * 
 * Important Settings:
 * ------------------
 * Recommended Partition Scheme (Tools -> Partition Scheme):
 *   - "32M Flash (4.8MB APP/22MB LittleFS)" - Best for ESP32-S3
 *   - "Default 4MB with ffat (1.2MB APP/1.5MB FATFS)" - Alternative option
 * 
 * Security Features:
 * -----------------
 * - IP-based access control
 * - Authentication required
 * - Attempt limiting with cooldown
 * - Automatic reboot after successful update
 * 
 * Libraries Required:
 * -----------------
 * - WiFi.h
 * - WebServer.h
 * - ElegantOTA
 */

/*
 * Software Setup:
 * - Install the ESP32 Board in Arduino IDE:
 *   - Open Arduino IDE.
 *   - Go to File > Preferences.
 *   - In the Additional Board Manager URLs field, add: https://dl.espressif.com/dl/package_esp32_index.json    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *   - Go to Tools > Board > Boards Manager.
 *   - Search for ESP32 and install esp32 by Espressif Systems.
 *   - Select ESP32S3 Dev Module
 * 
 * - Install Necessary Libraries:
 *   - Go to Sketch > Include Library > Manage Libraries.

 */

// The config.h file contains :
/* 
#ifndef CONFIG_H
#define CONFIG_H
// WiFi credentials
static const char* WIFI_SSID = "xx";
static const char* WIFI_PASSWORD = "xx";
// OTA Settings
#define OTA_USERNAME "xx"
#define OTA_PASSWORD "xx"
#define OTA_MAX_ATTEMPTS 3
#define OTA_BLOCK_TIME 300000  // 5 minutes
static const char* ALLOWED_IP = "192.xxx.x.xxx";
*/

#include "OTAManager.h"
#include "config.h"

WebServer server(80);
OTAManager otaManager(&server);

void setup() {
    Serial.begin(115200);
    
    // // WiFi configuration
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnecté au WiFi");
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.localIP());
    
    // OTA startup
    otaManager.begin();
    server.begin();

    // the rest of the code ....
}

void loop() {
    otaManager.loop();

    // the rest of the code ....
    //Serial.print("TEST2...");
    //delay(1000);
}
