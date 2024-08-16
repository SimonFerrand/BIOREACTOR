/*
 * ESP32 Code
 * 
 * This code receives data from the Arduino Mega via serial communication and sends it to a web server.
 * It also receives commands from the web server and sends them to the Arduino Mega.
 * 
 * Connections:
 * - RX (pin 18) of ESP32 to TX of Arduino Mega
 * - TX (pin 19) of ESP32 to RX of Arduino Mega
 * - GND of ESP32-S3 to GND of Arduino Mega
 * 
 * Libraries:
 * - ArduinoJson: To handle JSON parsing and serialization
 * - WiFi: To handle WiFi connections
 * - HTTPClient: To handle HTTP requests
 * - NTPClient: To get the current time
 * - WebSocketsClient: To handle WebSocket communication
 * - Crypto: To handle encryption and decryption
 * - AES: To use AES encryption
 * - config.h: Contains the WiFi and WebSocket server credentials, and the shared secret key
 * 
 * How it works:
 * - The ESP32 connects to the WiFi network.
 * - It connects to a WebSocket server to receive commands.
 * - When a command is received, it authenticates the command using the shared secret key.
 * - If the command is authenticated, it sends the corresponding command to the Arduino Mega via Serial2.
 * - When data is received from the Arduino Mega, it encrypts the data using the shared secret key and sends it to the web server using an HTTP POST request.
 * 
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
 *   - Search for and install ArduinoJson, WiFi, HTTPClient, NTPClient, WebSocketsClient, Crypto, and AES.
 *
 * - Partition Scheme
 *   - If you have space problems uploading the code, do the following: "Select Minimal SPIFFS (3.8MB APP with 256KB SPIFFS) in Tools > Partition Scheme".
 */

#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WebSocketsClient.h>
#include <Crypto.h>
#include <AES.h>
#include "config.h"

// The config.h file contains :
/* 
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";
const char* webSocketServer = "192.168.1.25";
const int webSocketPort = 8000;
const char* webSocketPath = "/ws";
const char* sharedSecret = "YourSecretKeyHere"; 
*/

// Define the pins for Serial2 communication with the Arduino Mega
const int rxPin = 18;
const int txPin = 19;

// Create an NTP client to get the current time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // Update the time every 60 seconds

// Create a WebSocket client to communicate with the WebSocket server
WebSocketsClient webSocket;

// Variable to keep track of the last time a message was received from the WebSocket server
unsigned long lastMessageTime = 0;

// Variables to handle WebSocket reconnection attempts
unsigned long lastWebSocketReconnectAttempt = 0;
const unsigned long webSocketReconnectInterval = 5000; // Attempt to reconnect every 5 seconds

// Function to handle WebSocket events
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  Serial.println("WebSocket event received");
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("WebSocket connected");
      break;
    case WStype_TEXT:
      Serial.printf("WebSocket received text: %s\n", payload);
      lastMessageTime = millis();
      handleCommand((char*)payload);
      break;
    case WStype_BIN:
      Serial.println("WebSocket received binary data");
      break;
    case WStype_PING:
      Serial.println("WebSocket received ping");
      break;
    case WStype_PONG:
      Serial.println("WebSocket received pong");
      break;
    default:
      Serial.printf("WebSocket received unhandled event type: %d\n", type);
      break;
  }
}

// Function to handle commands received from the WebSocket server
void handleCommand(const char* payload) {
  byte decryptedData[strlen(payload) + 1];
  AES aes;
  aes.set_key(reinterpret_cast<const byte*>(sharedSecret), sizeof(sharedSecret));
  int decryptedLen = aes.decrypt((byte*)payload, decryptedData);
  decryptedData[decryptedLen] = '\0';

  // Parse the decrypted JSON command
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, decryptedData);
  if (error) {
    Serial.println("Failed to parse JSON command");
    return;
  }

  String program = doc["program"];
  String command;

  if (program == "mix") {
    int speed = doc["speed"];
    command = "mix " + String(speed);
  } else if (program == "drain") {
    int rate = doc["rate"];
    int duration = doc["duration"];
    command = "drain " + String(rate) + " " + String(duration);
  } else if (program == "fermentation") {
    float temperature = doc["temperature"];
    float pH = doc["pH"];
    float dissolvedOxygen = doc["dissolvedOxygen"];
    float nutrientConcentration = doc["nutrientConcentration"];
    float baseConcentration = doc["baseConcentration"];
    int duration = doc["duration"];
    String experimentName = doc["experimentName"];
    String comment = doc["comment"];
    command = "fermentation " + String(temperature) + " " + String(pH) + " " + 
              String(dissolvedOxygen) + " " + String(nutrientConcentration) + " " + 
              String(baseConcentration) + " " + String(duration) + " " + 
              experimentName + " " + comment;
  } else if (program == "stop") {
    command = "stop";
  } else {
    Serial.println("Unknown program: " + program);
    return;
  }

  Serial2.println(command);
  Serial.println("Sent to Arduino: " + command);
}

void setup() {
  // Initialize the serial communication with the Arduino Mega
  Serial.begin(115200);
  delay(3000);
  Serial.println("ESP32-S3 Ready");
  Serial2.begin(9600, SERIAL_8N1, rxPin, txPin);
  Serial2.setRxBufferSize(256);
  Serial2.setTimeout(500);

  // Connect to the local WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString());

  // Initialize the NTP client to get the current time
  timeClient.begin();

  // Initialize the WebSocket connection
  webSocket.setExtraHeaders("X-Client-Type: ESP32");
  webSocket.begin(webSocketServer, webSocketPort, webSocketPath);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  static unsigned long lastCheck = 0;
  
  // Maintain the WebSocket connection
  webSocket.loop();

  // Check the WebSocket connection status periodically
  if (millis() - lastCheck > 5000) {
    lastCheck = millis();
    if (!webSocket.isConnected()) {
      Serial.println("WebSocket disconnected. Attempting to reconnect...");
      webSocket.disconnect();
      webSocket.begin(webSocketServer, webSocketPort, webSocketPath);
    } else {
      Serial.println("WebSocket is connected");
    }
    
    // Check if we've received a message from the WebSocket server recently
    if (millis() - lastMessageTime > 60000) {
      Serial.println("No WebSocket message received in the last minute");
    }
  }

  // Handle WiFi reconnection if the connection is lost
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWiFi reconnected");
    Serial.println("IP address: " + WiFi.localIP().toString());
  }

  // Process the data received from the Arduino Mega
  static String receivedData = "";
  while (Serial2.available()) {
    char incomingChar = Serial2.read();
    receivedData += incomingChar;

    if (incomingChar == '\n') {
      Serial.print("Received from Arduino Mega: ");
      Serial.println(receivedData);

      // Parse the received data and send it to the web server
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, receivedData);
      if (!error) {
        Serial.println("Valid JSON received");
        Serial.println(receivedData);

        // Prepare the JSON data to be sent to the web server
        if (doc.containsKey("ev") && doc["ev"] == "startup") {
          // Handle startup data
          doc["event"] = doc["ev"];
          doc["programType"] = doc["pt"];
          doc["rateOrSpeed"] = doc["rate"];
          doc["duration"] = doc["dur"];
          doc["tempSetpoint"] = doc["tSet"];
          doc["phSetpoint"] = doc["phSet"];
          doc["doSetpoint"] = doc["doSet"];
          doc["nutrientConc"] = doc["nutC"];
          doc["baseConc"] = doc["baseC"];
          doc["experimentName"] = doc["expN"];
          doc["comment"] = doc["comm"];
        } else {
          // Handle regular data
          doc["event"] = "data";
          doc["programType"] = doc["prog"];
          doc["rateOrSpeed"] = 0;
          doc["duration"] = 0;
          doc["tempSetpoint"] = 0.0;
          doc["phSetpoint"] = 0.0;
          doc["doSetpoint"] = 0.0;
          doc["nutrientConc"] = 0.0;
          doc["baseConc"] = 0.0;
          doc["experimentName"] = "";
          doc["comment"] = "";
        }

        // Add additional sensor data to the JSON document
        doc["currentProgram"] = doc["prog"];
        doc["programStatus"] = doc["stat"];
        doc["airPumpStatus"] = doc["ap"];
        doc["drainPumpStatus"] = doc["dp"];
        doc["nutrientPumpStatus"] = doc["np"];
        doc["basePumpStatus"] = doc["bp"];
        doc["stirringMotorStatus"] = doc["sm"];
        doc["heatingPlateStatus"] = doc["hp"];
        doc["ledGrowLightStatus"] = doc["lg"];
        doc["waterTemp"] = doc["wT"];
        doc["airTemp"] = doc["aT"];
        doc["ph"] = doc["pH"];
        doc["turbidity"] = doc["tb"];
        doc["oxygen"] = doc["ox"];
        doc["airFlow"] = doc["af"];

        // Encrypt the JSON document using the shared secret key
        byte encryptedData[receivedData.length() + 1];
        AES aes;
        aes.set_key(reinterpret_cast<const byte*>(sharedSecret), sizeof(sharedSecret));
        int encryptedLen = aes.encrypt((byte*)receivedData.c_str(), encryptedData);
        encryptedData[encryptedLen] = '\0';

        // Send the encrypted data to the web server
        if (WiFi.status() == WL_CONNECTED) {
          HTTPClient http;
          http.begin("http://192.168.1.25:8000/sensor_data");
          http.addHeader("Content-Type", "application/octet-stream");
          int httpResponseCode = http.POST(encryptedData, encryptedLen);
          if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Server response: " + response);
          } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
          }
          http.end();
        }
      } else {
        Serial.println("Invalid JSON format received: " + receivedData);
      }
      receivedData = "";
    }
  }
  
  delay(10); // Small delay to avoid overloading the CPU

  // Periodically log the status of the ESP32-S3 loop
  static unsigned long lastLog = 0;
  if (millis() - lastLog > 10000) {
    lastLog = millis();
    Serial.println("ESP32-S3 loop is running");
  }
}