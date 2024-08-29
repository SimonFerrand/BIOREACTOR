/*
  This code will control the LED light and fan based on commands received from the web interface or automatically according to the time schedule. The fan speed can be adjusted from 200 to 3300 RPM.


  # Instructions to connect the relay and control a 5V LED grow light (chip 2835) with ESP8266:

  1. Connections for the relay to ESP8266:
     - Relay VCC: Connect to 3.3V on the ESP8266.
     - Relay GND: Connect to GND on the ESP8266.
     - Relay IN (control signal): Connect to digital pin D1 (GPIO5) on the ESP8266.

  2. Connections for the LED light to the relay:
     - Red Wire (5V) from LED light: Connect to the normally open (NO) terminal of the relay.
     - Black Wire (GND) from LED light: Connect to the GND of the power source.
     - Power source 5V: Connect the positive terminal to the common (COM) terminal of the relay.
     - Power source GND: Connect the negative terminal to the GND of the ESP8266 and the GND of the LED light.

  3. Connections for the fan to ESP8266:
     - Fan VCC: Connect to 5V on the power source.
     - Fan GND: Connect to GND on the power source.
     - Fan PWM (speed control): Connect to digital pin D2 (GPIO4) on the ESP8266.
  
  Note: Ensure that your power source can provide enough current for both the LED light and the fan.


  # Installation instructions for ESP8266 with CH340 chip:

  1. Download and install the CH340 driver: 
     https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all

  2. Add the following URL to the Arduino IDE for additional boards manager URLs:
     http://arduino.esp8266.com/stable/package_esp8266com_index.json

  3. Install the ESP8266 board in the Arduino IDE boards manager:
     Go to "Tools" > "Board" > "Boards Manager", search for "esp8266", and install "esp8266 by ESP8266 Community".

  # The ESP-12F Module Capabilities:
  1. Wi-Fi: The ESP-12F has built-in Wi-Fi capabilities, allowing it to connect to Wi-Fi networks and act as an access point.
  2. GPIO: It has multiple General Purpose Input/Output (GPIO) pins for connecting sensors, LEDs, buttons, and other peripherals.
  3. PWM: Pulse Width Modulation for dimming LEDs or controlling servos.
  4. ADC: Analog to Digital Converter for reading analog sensors.
  5. I2C: Inter-Integrated Circuit for communication with other I2C devices.
  6. SPI: Serial Peripheral Interface for communication with SPI devices.
  7. UART: Universal Asynchronous Receiver/Transmitter for serial communication.
  8. Deep Sleep: Power-saving mode to reduce power consumption during inactivity.
*/

// ======= Libraries =======
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "config.h"

// ======= Pin Definitions =======
const int relayPin = 5; // GPIO5 (D1 on ESP8266)
const int builtInLed = 2; // Built-in LED (GPIO2)
const int fanPin = 4; // GPIO4 (D2 on ESP8266)

// ======= Network and Time Setup =======
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600 * 2, 1800000); // Update every half hour, GMT+2 for CEST
ESP8266WebServer server(80);

// ======= Time Control Settings =======
int onStartHour = 5; // 5 AM
int offStartHour = 21; // 9 PM

// ======= Control Variables =======
bool manualControl = false;
bool manualState = false;

// ======= Fan Control Settings =======
const int fanMinSpeed = 200; // Minimum RPM
const int fanMaxSpeed = 3300; // Maximum RPM
int targetFanSpeed = 400; // Target RPM

void setup() {
  Serial.begin(115200);
  delay(1000);  // Allow time for the serial connection to stabilize
  
  Serial.println("\n--- SETUP START ---");
  
  Serial.println("Initializing pins...");
  pinMode(relayPin, OUTPUT);
  pinMode(builtInLed, OUTPUT);
  pinMode(fanPin, OUTPUT);

  digitalWrite(relayPin, HIGH);
  digitalWrite(builtInLed, HIGH);

  Serial.println("Initializing fan speed...");
  updateFanSpeed();

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected successfully!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS IP: ");
  Serial.println(WiFi.dnsIP());

  Serial.println("Initializing NTP client...");
  timeClient.begin();
  timeClient.update();
  Serial.print("Current time: ");
  Serial.println(timeClient.getFormattedTime());

  Serial.println("Setting up web server...");
  setupServer();

  Serial.print("Fan speed set to ");
  Serial.print(targetFanSpeed);
  Serial.println(" RPM");

  Serial.println("--- SETUP COMPLETE ---\n");
}

void loop() {
  server.handleClient();
  timeClient.update();

  if (!manualControl) {
    int currentHour = (timeClient.getEpochTime() % 86400L) / 3600;
    if (currentHour >= onStartHour && currentHour < offStartHour) {
      digitalWrite(relayPin, LOW);
      digitalWrite(builtInLed, LOW);
    } else {
      digitalWrite(relayPin, HIGH);
      digitalWrite(builtInLed, HIGH);
    }
  }

  delay(100); // Short delay for stability
}

void setupServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/on", HTTP_GET, handleOn);
  server.on("/off", HTTP_GET, handleOff);
  server.on("/auto", HTTP_GET, handleAuto);
  server.on("/setfan", HTTP_GET, handleSetFan);
  server.on("/settime", HTTP_GET, handleSetTime);
  server.on("/status", HTTP_GET, handleStatus);
  server.begin();
  Serial.println("HTTP server started");
}

void handleRoot() {
  String html = "<html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>"
          "body { font-family: Arial, sans-serif; margin: 0 auto; max-width: 800px; padding: 20px; "
          "background-color: #222; color: #ddd; } "
          "h1, h2 { color: #fff; } "
          ".status { background-color: #333; padding: 10px; margin-bottom: 20px; border-radius: 5px; } "
          ".commands { background-color: #444; padding: 10px; margin-bottom: 20px; border-radius: 5px; } "
          "table { width: 100%; border-collapse: collapse; } "
          "th, td { border: 1px solid #555; padding: 8px; text-align: left; } "
          "th { background-color: #444; }"
          "a { color: #4CAF50; text-decoration: none; } "
          "a:hover { text-decoration: underline; } "
          "input[type='number'], input[type='submit'] { background-color: #333; color: #ddd; border: 1px solid #555; padding: 5px; }"
          "</style>";
  html += "</head><body>";
  html += "<h1>ESP8266 Control Panel</h1>";
  
  // Server functionality
  html += "<h2>Server Functionality:</h2>";
  html += "<ul>";
  html += "<li>Control LED Grow Light (ON/OFF/AUTO)</li>";
  html += "<li>Adjust fan speed</li>";
  html += "<li>Set LED on/off times</li>";
  html += "<li>Display current status and time</li>";
  html += "</ul>";
  
  // Current status
  html += "<h2>Current Status:</h2>";
  html += "<div class='status'>";
  html += "<p>Current time: " + timeClient.getFormattedTime() + "</p>";
  html += "<p>LED Grow Light: " + String(digitalRead(relayPin) == LOW ? "ON" : "OFF") + "</p>";
  html += "<p>Control mode: " + String(manualControl ? "Manual" : "Auto") + "</p>";
  html += "<p>Fan speed: " + String(targetFanSpeed) + " RPM</p>";
  html += "<p>LED ON time: " + String(onStartHour) + ":00</p>";
  html += "<p>LED OFF time: " + String(offStartHour) + ":00</p>";
  html += "</div>";
  
  // Controls
  html += "<h2>Controls:</h2>";
  html += "<p><a href='/on'>Turn LED ON</a> | <a href='/off'>Turn LED OFF</a> | <a href='/auto'>Set to AUTO mode</a></p>";
  html += "<form action='/setfan'>";
  html += "Set fan speed: <input type='number' name='speed' min='200' max='3300' value='" + String(targetFanSpeed) + "'>";
  html += "<input type='submit' value='Set'>";
  html += "</form>";
  html += "<form action='/settime'>";
  html += "Set LED ON time: <input type='number' name='on' min='0' max='23' value='" + String(onStartHour) + "'>";
  html += "Set LED OFF time: <input type='number' name='off' min='0' max='23' value='" + String(offStartHour) + "'>";
  html += "<input type='submit' value='Set'>";
  html += "</form>";
  
  // Available commands
  html += "<h2>Available Commands:</h2>";
  html += "<div class='commands'>";
  html += "<table>";
  html += "<tr><th>Command</th><th>Description</th></tr>";
  html += "<tr><td>/on</td><td>Turn LED Grow Light ON</td></tr>";
  html += "<tr><td>/off</td><td>Turn LED Grow Light OFF</td></tr>";
  html += "<tr><td>/auto</td><td>Set LED control to automatic mode</td></tr>";
  html += "<tr><td>/setfan?speed=X</td><td>Set fan speed to X RPM (200-3300)</td></tr>";
  html += "<tr><td>/settime?on=X&off=Y</td><td>Set LED ON time to X:00 and OFF time to Y:00 (0-23)</td></tr>";
  html += "<tr><td>/status</td><td>Get current status in plain text</td></tr>";
  html += "</table>";
  html += "</div>";
  
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSetTime() {
  if (server.hasArg("on") && server.hasArg("off")) {
    int newOnHour = server.arg("on").toInt();
    int newOffHour = server.arg("off").toInt();
    if (newOnHour >= 0 && newOnHour <= 23 && newOffHour >= 0 && newOffHour <= 23) {
      onStartHour = newOnHour;
      offStartHour = newOffHour;
      server.send(200, "text/plain", "LED times set to ON at " + String(onStartHour) + ":00 and OFF at " + String(offStartHour) + ":00");
    } else {
      server.send(400, "text/plain", "Invalid time values. Must be between 0 and 23.");
    }
  } else {
    server.send(400, "text/plain", "Missing on or off parameters");
  }
}

void handleOn() {
  manualControl = true;
  manualState = true;
  digitalWrite(relayPin, LOW);
  digitalWrite(builtInLed, LOW);
  server.send(200, "text/plain", "LED Grow Light turned ON");
}

void handleOff() {
  manualControl = true;
  manualState = false;
  digitalWrite(relayPin, HIGH);
  digitalWrite(builtInLed, HIGH);
  server.send(200, "text/plain", "LED Grow Light turned OFF");
}

void handleAuto() {
  manualControl = false;
  server.send(200, "text/plain", "Switched to automatic control");
}

void handleSetFan() {
  if (server.hasArg("speed")) {
    int speed = server.arg("speed").toInt();
    if (speed >= fanMinSpeed && speed <= fanMaxSpeed) {
      targetFanSpeed = speed;
      updateFanSpeed();
      server.send(200, "text/plain", "Fan speed set to " + String(targetFanSpeed) + " RPM");
    } else {
      server.send(400, "text/plain", "Invalid speed. Must be between " + String(fanMinSpeed) + " and " + String(fanMaxSpeed));
    }
  } else {
    server.send(400, "text/plain", "Missing speed parameter");
  }
}

void handleStatus() {
  String status = "Current time: " + timeClient.getFormattedTime() + "\n";
  status += "LED Grow Light: " + String(digitalRead(relayPin) == LOW ? "ON" : "OFF") + "\n";
  status += "Control mode: " + String(manualControl ? "Manual" : "Auto") + "\n";
  status += "Fan speed: " + String(targetFanSpeed) + " RPM\n";
  server.send(200, "text/plain", status);
}

void updateFanSpeed() {
  int pwmValue = map(targetFanSpeed, fanMinSpeed, fanMaxSpeed, 0, 1023);
  analogWrite(fanPin, pwmValue);
}
