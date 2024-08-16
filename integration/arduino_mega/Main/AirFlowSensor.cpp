/*
 * AirFlowSensor.cpp
 * This file provides the implementation of the AirFlowSensor class defined in AirFlowSensor.h.
 * The class reads the air flow rate from a YF-S401 air flow meter.
 */

#include "AirFlowSensor.h"
#include "Logger.h"

const float AirFlowSensor::_pulsesPerLiter = 5880.0; // Pulses per liter as per the sensor's specification
AirFlowSensor* AirFlowSensor::instance = nullptr; // Initialize the static instance

// Constructor for AirFlowSensor
AirFlowSensor::AirFlowSensor(int pin, const char* name) : _pin(pin), _pulseCount(0), _lastTime(0), _name(name)  {
    instance = this; // Set the instance to this object
}

// Method to initialize the air flow meter sensor
void AirFlowSensor::begin() {
    pinMode(_pin, INPUT_PULLUP); // Set the flow meter pin as input with internal pull-up resistor
    attachInterrupt(digitalPinToInterrupt(_pin), countPulses, FALLING); // Attach interrupt to count pulses
    Logger::log(LogLevel::INFO, String(_name) + " initialized");
}

// Method to read the flow rate from the sensor
float AirFlowSensor::readValue() {
    unsigned long currentTime = millis();

    // Calculate flow rate every '_interval' milliseconds
    if (currentTime - _lastTime >= _interval) {
        noInterrupts(); // Disable interrupts while calculating
        unsigned long pulses = _pulseCount;
        _pulseCount = 0; // Reset pulse count
        interrupts(); // Enable interrupts

        // Flow rate calculation
        float flowRate = (pulses / _pulsesPerLiter) * 60.0; // Convert pulses to liters per minute

        _lastTime = currentTime;
        return flowRate;
    }
    return -1.0; // Indicate that the flow rate is not updated yet
}

// Interrupt service routine to count pulses
void AirFlowSensor::countPulses() {
    instance->_pulseCount++;
}
