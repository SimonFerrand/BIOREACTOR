/*
 * PT100Sensor.cpp
 * This file provides the implementation of the PT100Sensor class defined in PT100Sensor.h.
 * The class reads temperature values from a PT100 sensor using the MAX31865 module.
 */

#include "PT100Sensor.h"
#include "Logger.h"

// Constructor for PT100Sensor
PT100Sensor::PT100Sensor(int csPin, int diPin, int doPin, int clkPin, const char* name)
    : _thermo(csPin, diPin, doPin, clkPin), _name(name) {}

// Method to initialize the PT100 sensor
void PT100Sensor::begin() {
    _thermo.begin(MAX31865_3WIRE); // Set up the MAX31865 module for 3-wire RTD
    //Logger::log(LogLevel::INFO, String(_name) + " initialized");
    Logger::log(LogLevel::INFO, String(_name) + F(" initialized"));
}

// Method to read the temperature from the sensor
float PT100Sensor::readValue() {
    return _thermo.temperature(100, 430); // Parameters are Rref and RTD nominal resistance
}
