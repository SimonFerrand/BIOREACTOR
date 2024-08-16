/*
 * LEDGrowLight.cpp
 * This file provides the implementation of the LEDGrowLight class defined in LEDGrowLight.h.
 * The class controls a LED grow light using a relay and potentially PWM for intensity control.
 */

#include "LEDGrowLight.h"
#include "Logger.h"

// Constructor for LEDGrowLight
LEDGrowLight::LEDGrowLight(int relayPin, const char* name)
    : _relayPin(relayPin), _name(name), status(false), intensity(0) {
    pinMode(_relayPin, OUTPUT); // Set relay pin as output
    digitalWrite(_relayPin, LOW); // Ensure relay is off initially
}

void LEDGrowLight::begin() {
    digitalWrite(_relayPin, LOW); // Ensure the LED grow light is off at initialization
    Logger::log(LogLevel::INFO, String(_name) + " initialized");
}

// Method to control the LED grow light
void LEDGrowLight::control(bool state, int value) {
    if (state) {
        intensity = constrain(value, 0, 100); // Constrain intensity to 0-100%
        digitalWrite(_relayPin, HIGH); // Turn on the relay
        status = true; // Set the status to on
        Logger::log(LogLevel::INFO, String(_name) + " is ON with intensity: " + String(intensity) + "%");
    } else {
        digitalWrite(_relayPin, LOW); // Turn off the relay
        status = false; // Set the status to off
        intensity = 0; // Reset intensity to 0
        Logger::log(LogLevel::INFO, String(_name) + " is OFF");
    }
}

// Method to check if the LED grow light is on
bool LEDGrowLight::isOn() const {
    return status;
}