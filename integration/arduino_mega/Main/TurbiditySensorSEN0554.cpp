/*
 * TurbiditySensorSEN0554.cpp
 * This file provides the implementation of the TurbiditySensorSEN0554 class defined in TurbiditySensorSEN0554.h.
 * The class reads turbidity values from a UART non-contact optical turbidity sensor (SEN0554).
 *
 * Reference: https://wiki.dfrobot.com/SKU_SEN0554_Turbidity_Sensor
 */

#include "TurbiditySensorSEN0554.h"
#include "Logger.h"

TurbiditySensorSEN0554::TurbiditySensorSEN0554(int rxPin, int txPin, const char* name)
    : _serial(rxPin, txPin), _name(name) {}

void TurbiditySensorSEN0554::begin() {
    _serial.begin(9600);
    //Logger::log(LogLevel::INFO, String(_name) + " initialized");
    Logger::log(LogLevel::INFO, String(_name) + F(" initialized"));
}

float TurbiditySensorSEN0554::readValue() {
    if (communicate()) {
        int rawTurbidity = _response[3];
        float adjustedTurbidity = (rawTurbidity * SCALE_FACTOR) + OFFSET;
        //Logger::log(LogLevel::INFO, String(_name) + " - raw value: " + String(rawTurbidity) + ", Adjusted value: " + String(adjustedTurbidity));     
        return adjustedTurbidity;
    }
    //Logger::log(LogLevel::WARNING, String(_name) + " - No sensor response");
    Logger::log(LogLevel::WARNING, String(_name) + F(" - No sensor response"));
    return -1.0f;
}

bool TurbiditySensorSEN0554::communicate() {
    _serial.write(_command, 5);
    delay(100);  // Délai ajouté comme dans le code fonctionnel

    if (_serial.available() >= 5) {
        for (int i = 0; i < 5; i++) {
            _response[i] = _serial.read();
        }
        
        if (_response[0] == 0x18 && _response[1] == 0x05) {
            return true;
        } else {
            Logger::log(LogLevel::ERROR, String(_name) + " - Réponse invalide du capteur");
            Logger::log(LogLevel::ERROR, String(_name) + F(" - Réponse invalide du capteur"));
        }
    }
    return false;
}