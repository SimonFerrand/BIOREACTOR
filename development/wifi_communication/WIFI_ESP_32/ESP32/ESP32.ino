/*
 * ESP32 Code
 * 
 * This code receives data from the Arduino Uno via serial communication and sends it to a web server.
 * 
 * Connections:
 * - RX (pin 16) of ESP32 to TX of Arduino Uno
 * - TX (pin 17) of ESP32 to RX of Arduino Uno
 * - GND of ESP32 to GND of Arduino Uno
 * 
 * Libraries:
 * - WiFi: To handle WiFi connections
 * - HTTPClient: To handle HTTP requests
 * - config.h: Contains the WiFi credentials
 * 
 * How it works:
 * - The ESP32 connects to the WiFi network.
 * - It waits for data from the Arduino Uno via Serial2.
 * - When data is received, it sends the data to a web server using an HTTP POST request.
 * - It sends a confirmation message back to the Arduino Uno.
 * 
 * Software Setup:
 * - Install the UART Driver (if needed):
 *   - Download and install the UART driver from Silabs USB to UART Bridge VCP Drivers.
 *   - URL: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads
 *   - Connect your ESP32
 *   - In Device Manager, find the unrecognized device or one with an exclamation mark under Ports (COM & LPT) or Other Devices.
 *   - Right-click the device, select Update driver, then choose Browse my computer for drivers.
 * 
 * - Install the ESP32 Board in Arduino IDE:
 *   - Open Arduino IDE.
 *   - Go to File > Preferences.
 *   - In the Additional Board Manager URLs field, add: https://dl.espressif.com/dl/package_esp32_index.json.
 *   - Go to Tools > Board > Boards Manager.
 *   - Search for ESP32 and install esp32 by Espressif Systems.
 * 
 * - Install Necessary Libraries:
 *   - Go to Sketch > Include Library > Manage Libraries.
 *   - Search for and install WiFi and HTTPClient.
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"

// Define the pins for Serial2
const int rxPin = 16;
const int txPin = 17;

void setup() {
  // Initialize serial communication with the computer and the Arduino Uno
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, rxPin, txPin);

  Serial.println("ESP32 ready");
  delay(1000); // Time to stabilize

  // Connect to the WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Wait for a message from the Arduino Uno
  if (Serial2.available()) {
    String arduinoMessage = Serial2.readStringUntil('\n');
    Serial.println("Value received from Arduino Uno: " + arduinoMessage);

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin("https://bioreactor.ddns.net/server.php");
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String postData = "sensor_value=" + arduinoMessage;
      int httpResponseCode = http.POST(postData);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Server response: " + response);
        Serial2.println("Received: " + arduinoMessage); // Confirmation to the Arduino Uno
      } else {
        Serial.println("Error connecting to server");
        Serial2.println("Error"); // Indicate an error to the Arduino Uno
      }
      http.end();
    } else {
      Serial.println("WiFi disconnected");
    }
  }
  delay(1000); // Wait before checking again
}
