/*
 * ESP32 Code
 * 
 * This code handles communication between a Teensy (main controller) and a web server running on a Raspberry Pi,
 * using WebSocket and HTTP communications. It receives data from the Teensy 
 * via serial communication and sends it to the web server. It also receives commands 
 * from the web server via WebSocket and sends them to the Teensy.
 * 
 * Connections:
 * - RX (pin 12) of ESP32 to TX of Teensy
 * - TX (pin 14) of ESP32 to RX of Teensy
 * - GND of ESP32 to GND of Teensy
 * 
 * Libraries:
 * - ArduinoJson: To handle JSON parsing and serialization
 * - WiFi: To handle WiFi connections
 * - HTTPClient: To handle HTTP requests
 * - AsyncMqttClient: To handle Mqtt communication
 * - ezTime: To handle time synchronization and formatting
 * - config.h: Contains the WiFi and WebSocket server credentials
 * 
 * How it works:
 * - The ESP32 connects to the WiFi network.
 * - It connects to a WebSocket server on the Raspberry Pi to receive commands.
 * - When a command is received, it sends the corresponding command to the Teensy via Serial2.
 * - When data is received from the Teensy, it sends it to the web server using an HTTP POST request.
 * 
 * Software Setup:
 * - Install the ESP32 Board in Arduino IDE:
 *   - Open Arduino IDE.
 *   - Go to File > Preferences.
 *   - In the Additional Board Manager URLs field, add: https://dl.espressif.com/dl/package_esp32_index.json    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *   - Go to Tools > Board > Boards Manager.
 *   - Search for ESP32 and install esp32 by Espressif Systems.
 *   - Select ESP32 Dev Module
 * 
 * - Install Necessary Libraries:
 *   - Go to Sketch > Include Library > Manage Libraries.
 *   - Search for and install ArduinoJson, WiFi, HTTPClient, WebSocketsClient, and ezTime.
 *
 * - Partition Scheme
 *   - If you have space problems uploading the code, do the following: "Select Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS) in Tools > Partition Scheme".
 *
 * Note: Voltage matching: If you are communicating between two boards operating at different voltages 
 * (for example, 3.3V for the ESP32 and 5V for the Teensy), you need to match the voltage levels. 
 * By using a resistor or voltage divider, you protect the ESP32 circuit from overvoltages that could damage the pins.
 * You can also use a bidirectional logic level converter to safely interface between the two voltage levels.
 *
 * A USB isolator like the DSD TECH SH-G01A with ADUM3160 12M chip can be used to provide electrical isolation. 
 * This type of module prevents current from flowing back to the computer, protecting against 
 * ground loops and potential differences. It converts the USB signals optically or magnetically, 
 * ensuring that there's no direct electrical connection between the two sides.
 *
 * **ATTENTION: The USB isolator may not provide sufficient power for the ESP32 to operate its WiFi functionality. 
 * In this case, it's necessary to power the ESP32 externally and use the isolator only for serial communication.**
 */

 /*
// config.h
#ifndef CONFIG_H
#define CONFIG_H
// WiFi credentials
const char ssid[] = "xx;
const char password[] = "xxx";
// Authentication variable
const uint8_t sharedSecret[] = { xxx };
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
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <AsyncMqttClient.h> //MQTT   https://github.com/marvinroger/async-mqtt-client
#include <ezTime.h>
#include "config.h"

// Define the pins for Serial2 communication with the Teensy
const int rxPin = 12;
const int txPin = 14;

// MQTT client
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

// Time management
Timezone myTZ;

void printMqttDisconnectReason(AsyncMqttClientDisconnectReason reason) {
    const char* reasonString;
    switch (static_cast<uint8_t>(reason)) {
        case 0: reasonString = "TCP Disconnected"; break;
        case 1: reasonString = "MQTT Unacceptable Protocol"; break;
        case 2: reasonString = "MQTT Identifier Rejected"; break;
        case 3: reasonString = "MQTT Server Unavailable"; break;
        case 4: reasonString = "MQTT Bad User/Pass"; break;
        case 5: reasonString = "MQTT Not Authorized"; break;
        default: reasonString = "Unknown";
    }
    Serial.printf("Disconnected from MQTT, reason: %s (%d)\n", reasonString, static_cast<uint8_t>(reason));
}

void connectToWifi() {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
}

void connectToMqtt() {
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch(event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0);
        xTimerStart(wifiReconnectTimer, 0);
        break;
    }
}

void onMqttConnect(bool sessionPresent) {
    Serial.println("Connected to MQTT.");
    Serial.printf("Session present: %d\n", sessionPresent);
    
    // Subscribe to command topic
    uint16_t packetIdSub = mqttClient.subscribe(MQTT_COMMAND_TOPIC, 2);
    Serial.printf("Subscribing to %s at QoS 2, packetId: %d\n", MQTT_COMMAND_TOPIC, packetIdSub);
    
    // Publish connection status
    String status = "{\"device\":\"ESP32\",\"status\":\"connected\",\"ip\":\"" + WiFi.localIP().toString() + "\"}";
    mqttClient.publish(MQTT_STATUS_TOPIC, 0, true, status.c_str());
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    printMqttDisconnectReason(reason);
    if (WiFi.isConnected()) {
        xTimerStart(mqttReconnectTimer, 0);
    }
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, 
                   size_t len, size_t index, size_t total) {
    String message = String(payload, len);
    Serial.printf("Received MQTT message on topic: %s\n", topic);
    Serial.println(message);

    if (String(topic) == MQTT_COMMAND_TOPIC) {
        // Forward command to Teensy
        Serial.println("Forwarding command to Teensy:");
        Serial.println(message);
        Serial2.println(message);
    }
}

void onMqttPublish(uint16_t packetId) {
    Serial.printf("Publish acknowledged, packetId: %d\n", packetId);
}

void setup() {
    Serial.begin(115200);
    delay(3000);
    Serial.println("ESP32 Ready");

    // Setup Serial2 communication with Teensy
    Serial2.begin(9600, SERIAL_8N1, rxPin, txPin);
    Serial2.setRxBufferSize(256);
    Serial2.setTimeout(500);
    
    // Create timers for reconnection
    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, 
                                     reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
    wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, 
                                     reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

    // Setup WiFi
    WiFi.onEvent(WiFiEvent);

    // Configure MQTT
    IPAddress mqtt_server;
    if(!mqtt_server.fromString(MQTT_HOST)) {
        Serial.println("Failed to parse MQTT host");
    } else {
        Serial.print("MQTT Server IP: ");
        Serial.println(mqtt_server.toString());
    }
    
    mqttClient.setServer(mqtt_server, MQTT_PORT);
    mqttClient.setClientId(MQTT_CLIENT_ID);
    mqttClient.setKeepAlive(15);
    mqttClient.setCleanSession(true);
    
    // Set MQTT callbacks
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);

    // Initial connection
    connectToWifi();
    
    // Setup time
    waitForSync();
    myTZ.setLocation(F("Europe/Paris"));
}

void loop() {
    // Process data from Teensy
    static String receivedData = "";
    while (Serial2.available()) {
        char incomingChar = Serial2.read();
        receivedData += incomingChar;

        if (incomingChar == '\n') {
            Serial.print("Received from Teensy: ");
            Serial.println(receivedData);

            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("Preparing HTTP request...");
                // Prepare JSON data
                String jsonData = "{\"arduino_value\":" + receivedData.substring(0, receivedData.length() - 1) +
                                ",\"timestamp\":\"" + myTZ.dateTime("H:i:s") + "\"}";
                Serial.println("JSON data: " + jsonData);

                // Send to HTTP endpoint
                HTTPClient http;
                Serial.println("Sending HTTP POST to server...");
                http.begin("http://192.168.1.25:8000/sensor_data");
                http.addHeader("Content-Type", "application/json");
                
                int httpResponseCode = http.POST(jsonData);
                if (httpResponseCode > 0) {
                    String response = http.getString();
                    Serial.println("Server response: " + response);
                } else {
                    Serial.print("HTTP Error: ");
                    Serial.println(httpResponseCode);
                }
                http.end();

                // Send to MQTT if connected
                if (mqttClient.connected()) {
                    mqttClient.publish(MQTT_SENSOR_TOPIC, 0, false, jsonData.c_str());
                    Serial.println("Data sent to MQTT");
                }
            }

            receivedData = "";
        }
    }

    // Status logging
    static unsigned long lastStatusCheck = 0;
    if (millis() - lastStatusCheck > 10000) {  // Every 10 seconds
        lastStatusCheck = millis();
        Serial.println("ESP32 Status:");
        Serial.printf("WiFi Connected: %d\n", WiFi.status() == WL_CONNECTED);
        Serial.printf("MQTT Connected: %d\n", mqttClient.connected());
        Serial.println("ESP32 loop is running");
    }

    delay(10);  // Prevent watchdog issues
}