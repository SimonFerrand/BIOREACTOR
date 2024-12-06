// DataProvider.cpp - Exemple d'implémentation
#include "DataProvider.h"
#include <WiFi.h>
#include "config.h"

SensorData BioreactorDataProvider::generateRandomData() {
    SensorData data;
    data.waterTemp = random(2000, 3000) / 100.0f;
    data.airTemp = random(1800, 2800) / 100.0f;
    data.pH = random(680, 720) / 100.0f;
    data.oxygen = random(800, 950) / 10.0f;
    data.turbidity = random(0, 1000) / 10.0f;
    return data;
}

SensorData BioreactorDataProvider::getLatestSensorData() {
    // À adapter selon votre système
    return generateRandomData();
}

String BioreactorDataProvider::getSystemStatus() {
    return "Connected - Running";
}

String BioreactorDataProvider::getDeviceInfo() {
    String info;
    info += "Device ID: " + String(MQTT_CLIENT_ID) + "<br>";
    info += "IP: " + WiFi.localIP().toString() + "<br>";
    info += "Uptime: " + String(millis() / 1000) + "s<br>";
    info += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes<br>";
    return info;
}