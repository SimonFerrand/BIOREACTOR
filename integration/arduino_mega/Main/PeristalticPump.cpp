/*
 * PeristalticPump.cpp
 * This file provides the implementation of the PeristalticPump class defined in PeristalticPump.h.
 * The class controls a peristaltic pump using a DAC (MCP4725) and a relay.
 */

#include "PeristalticPump.h"
#include "Logger.h"

// Constructor for PeristalticPump
PeristalticPump::PeristalticPump(uint8_t dacAddress, int relayPin, float minFlowRate, float maxFlowRate, const char* name)
    : _dacAddress(dacAddress), _relayPin(relayPin), _minFlowRate(minFlowRate), _maxFlowRate(maxFlowRate), _name(name), _status(false), _currentFlowRate(0), _volumeAdded(0) {
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
    float _flowRate = constrain(value, _minFlowRate, _maxFlowRate);

    if (state != _status || (state && _flowRate != _currentFlowRate)) {
        if (state && _flowRate > _minFlowRate) {
            uint16_t dacValue = flowRateToDAC(_flowRate);
            _dac.setVoltage(dacValue, false);
            digitalWrite(_relayPin, HIGH);
            _status = true;
            _currentFlowRate = _flowRate;
            float duration = 1.0 / 60.0;
            _volumeAdded += _flowRate * duration;
            Logger::log(LogLevel::INFO, String(_name) + F(" is ON with flow rate: ") + String(_flowRate) + F(" ml/min"));
        } else {
            _dac.setVoltage(0, false);
            digitalWrite(_relayPin, LOW);
            _status = false;
            _currentFlowRate = 0;
            //Logger::log(LogLevel::INFO, String(_name) + " is OFF");
            Logger::log(LogLevel::INFO, String(_name) + F(" is OFF"));
        }
    }
}

// Method to check if the pump is on
bool PeristalticPump::isOn() const {
    return _status;
}

// Converts flow rate in ml/min to DAC value
uint16_t PeristalticPump::flowRateToDAC(float _flowRate) {
    float proportion = _flowRate / _maxFlowRate;       // Calculate proportion of max flow rate
    return static_cast<uint16_t>(proportion * 4095);  // Convert to DAC value (0-4095)
}
