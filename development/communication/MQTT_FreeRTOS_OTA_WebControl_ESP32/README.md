// ===== README.md =====
# ESP32 MQTT Bioreactor Control System

A modular ESP32-based system for controlling and monitoring a bioreactor using MQTT communication and Web Interface.

## Features

- Real-time sensor monitoring (temperature, pH, oxygen, turbidity)
- MQTT communication with auto-reconnection
- Web Interface with real-time updates
- FreeRTOS task management
- System monitoring (memory, WiFi signal)
- Structured logging

## Prerequisites

### Hardware
- ESP32 development board
- Sensors (temperature, pH, oxygen, turbidity)

### Software
- Arduino IDE 2.0 or later
- Required libraries:
  - AsyncMqttClient
  - ArduinoJson
  - WebServer
  - ElegantOTA
  - WiFi (ESP32)

## Installation

1. Clone the repository:
```bash
git clone https://github.com/yourusername/esp32-bioreactor.git
```

2. Copy `private_config.h.example` to `private_config.h` and update your credentials:
```cpp
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
#define MQTT_CLIENT_ID "esp32_bioreactor"
```

3. Configure your MQTT broker settings in `config.h`:
```cpp
#define MQTT_BROKER "192.168.1.25"
#define MQTT_PORT 1883
```

4. Install required libraries in Arduino IDE:
   - Tools -> Manage Libraries
   - Search and install all required libraries

5. Get your ESP32's IP address from your router after first connection
6. Set a static IP for your ESP32 in your router settings
7. Update ALLOWED_IP in config.h with this static IP to enable OTA updates:
   ```cpp
   static const char* ALLOWED_IP = "192.168.1.xxx"; // Your ESP32's static IP
   ```

## Project Structure

```
project/
├── Logger.h         # Logging system
├── TaskManager.h    # FreeRTOS management
├── SystemMonitor.h  # Resource monitoring
├── WiFiManager.h    # Connection handling
├── MQTTClient.h     # MQTT communication
├── WebServerManager.h # Web & API server
├── MessageFormatter.h # Data formatting
├── DataProvider.h    # Sensor interface
├── config.h         # Public configuration
└── private_config.h  # Credentials
```

## MQTT Topics

The system provides a built-in web interface accessible via the ESP32's IP address:

- `/`: Main dashboard with real-time data
- `/data`: Detailed sensor data view
- `/api/data`: JSON API endpoint for sensor data

## MQTT Topics

- `bioreactor/status`: Device status updates
- `bioreactor/sensors`: Sensor data
- `bioreactor/commands`: Command channel




