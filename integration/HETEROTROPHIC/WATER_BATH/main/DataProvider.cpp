// DataProvider.cpp - Exemple d'implémentation
#include "DataProvider.h"
#include <WiFi.h>
#include "config.h"

SensorData BioreactorDataProvider::getLatestSensorData() {
    SensorData data;
    data.waterTemp = SensorController::readSensor("waterTempSensor");
    return data;
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