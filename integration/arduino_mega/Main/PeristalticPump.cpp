/*
 * PeristalticPump.cpp
 * This file provides the implementation of the PeristalticPump class defined in PeristalticPump.h.
 * The class controls a peristaltic pump using a DAC (MCP4725) and a relay.
 */

#include "PeristalticPump.h"
#include "Logger.h"

// Constructor for PeristalticPump
PeristalticPump::PeristalticPump(uint8_t dacAddress, int relayPin, float minFlowRate, float maxFlowRate, const char* name)
    : _dacAddress(dacAddress), _relayPin(relayPin), _minFlowRate(minFlowRate), _maxFlowRate(maxFlowRate), _name(name), status(false), volumeAdded(0) {
}

// Initializes the peristaltic pump by setting up the relay pin and the DAC
void PeristalticPump::begin() {
    pinMode(_relayPin, OUTPUT);      // Set relay pin as output
    digitalWrite(_relayPin, LOW);    // Ensure relay is off initially
    _dac.begin(_dacAddress);         // Initialize the DAC with its I2C address
    //Logger::log(LogLevel::INFO, String(_name) + " initialized");
    Logger::log(LogLevel::INFO, String(_name) + F(" initialized"));
}

// Controls the pump's state and flow rate
void PeristalticPump::control(bool state, int value) {
    if (state && value > _minFlowRate) {
        float flowRate = constrain(value, _minFlowRate, _maxFlowRate);
        uint16_t dacValue = flowRateToDAC(flowRate);
        _dac.setVoltage(dacValue, false);
        digitalWrite(_relayPin, HIGH);
        status = true;
        float duration = 1.0 / 60.0;
        volumeAdded += flowRate * duration;
        Logger::log(LogLevel::INFO, String(_name) + " is ON with flow rate: " + String(flowRate) + " ml/min");
    } else {
        _dac.setVoltage(0, false);
        digitalWrite(_relayPin, LOW);
        status = false;
        //Logger::log(LogLevel::INFO, String(_name) + " is OFF");
        Logger::log(LogLevel::INFO, String(_name) + F(" is OFF"));
    }
}

// Method to check if the pump is on
bool PeristalticPump::isOn() const {
    return status;
}

// Converts flow rate in ml/min to DAC value
uint16_t PeristalticPump::flowRateToDAC(float flowRate) {
    float proportion = flowRate / _maxFlowRate;       // Calculate proportion of max flow rate
    return static_cast<uint16_t>(proportion * 4095);  // Convert to DAC value (0-4095)
}
