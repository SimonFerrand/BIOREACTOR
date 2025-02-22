/*
 * DCPump.cpp
 * This file provides the implementation of the DCPump class defined in DCPump.h.
 * The class controls a DC pump using PWM signals, a relay, and a motor speed controller.
 * It allows for setting the pump speed and turning it on or off.
 */

#include "DCPump.h"
#include "Logger.h"

// Constructor for DCPump
DCPump::DCPump(int channel, int relayPin, int minPWM, const char* name)
    : _channel(channel), _relayPin(relayPin), _minPWM(minPWM), _name(name), _status(false),  _currentValue(0), _volumeAdded(0) {
    pinMode(_relayPin, OUTPUT); // Set relay pin as output
}

// Method to initialize the pump
void DCPump::begin() {
    // Ensure the pump is off at initialization
    digitalWrite(_relayPin, LOW);
    analogWrite(_channel, 0);
    //Logger::log(LogLevel::INFO, String(_name) + " initialized");
    Logger::log(LogLevel::INFO, String(_name) + F(" initialized"));
}

// Method to control the pump
void DCPump::control(bool state, int value) {
    value = constrain(value, 0, 100);

    if (state != _status || (state && value != _currentValue)) {
        if (state && value >= _minPWM) {
            uint16_t dacValue = (value * 4095 * 5.0) / (100 * 5.4);  // Convert percentage (0-100%) to DAC value (0-4095) with voltage compensation from 5.4V to 5.0V
            QuadChannelDACController::getInstance().setVoltage(_channel, dacValue); // Set the analogic value
            digitalWrite(_relayPin, HIGH); // Turn on the relay
            _status = true; // Set the status to on
            _currentValue = value;
            //Logger::log(LogLevel::INFO, String(_name) + " is ON, Speed set to: " + String(value));
            //Logger::log(LogLevel::INFO, String(_name) + F(" is ON, Speed set to: ") + String(value)+ F("%"));

            float flowRate = value * 0.1; // Example: 0.1 ml/min per unit of value
            float duration = 1.0 / 60.0; // 1 second in minutes
            _volumeAdded += (flowRate /1000) * duration; // convertir in liters
        } else {
            QuadChannelDACController::getInstance().setVoltage(_channel, 0); // Set analogic value to 0
            digitalWrite(_relayPin, LOW); // Turn off the relay
            _status = false; // Set the status to off
            _currentValue = 0;
            //Logger::log(LogLevel::INFO, String(_name) + " is OFF");
            //Logger::log(LogLevel::INFO, String(_name) + F(" is OFF"));
        }
    }
}

// Method to check if the pump is on
bool DCPump::isOn() const {
    return _status;
}

int DCPump::getCurrentValue() const {
    return _currentValue;
}