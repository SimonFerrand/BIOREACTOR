/*
 * StirringMotor.cpp
 * This file provides the implementation of the StirringMotor class defined in StirringMotor.h.
 * The class controls a stirring motor (fan) using PWM signals.
 */

#include "StirringMotor.h"
#include "Logger.h"

// Constructor for StirringMotor
StirringMotor::StirringMotor(int pwmPin, int relayPin, int minRPM, int maxRPM, const char* name)
    : _pwmPin(pwmPin), _relayPin(relayPin), _name(name), _status(false), _currentRPM(0), _minRPM(minRPM), _maxRPM(maxRPM) {
}

void StirringMotor::begin() {
    pinMode(_pwmPin, OUTPUT);
    pinMode(_relayPin, OUTPUT);
    digitalWrite(_relayPin, LOW);
    analogWrite(_pwmPin, 0);
    //Logger::log(LogLevel::INFO, String(_name) + " initialized");
    Logger::log(LogLevel::INFO, String(_name) + F(" initialized"));
}

// Method to control the stirring motor
void StirringMotor::control(bool state, int value) {
    int _targetRPM = constrain(value, _minRPM, _maxRPM);

    if (state != _status || (state && _targetRPM != _currentRPM)) {
        if (state && _targetRPM > 0) {
            int pwmValue = rpmToPWM(_targetRPM);
            analogWrite(_pwmPin, pwmValue);
            digitalWrite(_relayPin, HIGH);
            _status = true;
            _currentRPM = _targetRPM;
            Logger::log(LogLevel::INFO, String(_name) + F(" is ON, RPM set to: ") + String(_targetRPM));
        } else {
            analogWrite(_pwmPin, 0);
            digitalWrite(_relayPin, LOW);
            _status = false;
            _currentRPM = 0;
            Logger::log(LogLevel::INFO, String(_name) + F(" is OFF"));
        }
    }
}

// Method to check if the motor is on
bool StirringMotor::isOn() const {
    return _status;
}

// Method to convert RPM to PWM value
int StirringMotor::rpmToPWM(int _targetRPM) {
    float loadPercentage; // Variable to hold the calculated load percentage

    // Calculate the load percentage based on the target RPM using piecewise linear equations
    if (_targetRPM <= 450) {
        // First equation for RPMs from 0% to 32% load
        loadPercentage = (_targetRPM - 390) / 1.875;
    } else {
        // Second equation for RPMs from 32% to 100% load
        loadPercentage = (_targetRPM - 450) / 15.441 + 32;
    }

    // Convert the load percentage to a PWM value (range 0 to 255)
    int pwmValue = map(int(loadPercentage), 0, 100, 0, 255);
    return pwmValue;
}