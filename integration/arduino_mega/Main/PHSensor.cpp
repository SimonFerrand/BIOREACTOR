/*
 * PHSensor.cpp
 * This file provides the implementation of the PHSensor class defined in PHSensor.h.
 * The class reads pH value from a pH sensor and performs calibration.
 */

#include "PHSensor.h"
#include "Logger.h"

// Constructor for PHSensor
PHSensor::PHSensor(int pin, PT100Sensor* tempSensor, const char* name) 
    : _pin(pin), _tempSensor(tempSensor), _name(name), _voltage(0) {}

// Method to initialize the pH sensor
void PHSensor::begin() {
    _ph.begin();
    //Logger::log(LogLevel::INFO, String(_name) + " initialized");
    Logger::log(LogLevel::INFO, String(_name) + F(" initialized"));
}

// Method to read the pH value from the sensor
float PHSensor::readValue() {
    _voltage = analogRead(_pin) / 1024.0 * 5000; // Convert analog reading to millivolts
    float temperature = _tempSensor->readValue();
    return _ph.readPH(_voltage, _temperature); // Calculate pH value with temperature compensation
}

// Method to handle pH calibration commands
void PHSensor::calibration(const char* cmd) {
    float temperature = _tempSensor->readValue();
    _ph.calibration(_voltage, _temperature, const_cast<char*>(cmd)); // Call the calibration method from DFRobot_PH class
}