/*
 * HeatingPlate.cpp
 * This file provides the implementation of the HeatingPlate class defined in HeatingPlate.h.
 * The class controls a heating plate using either on/off control or PWM control.
 */

#include "HeatingPlate.h"
#include "Logger.h"
#include <Arduino.h>

/*
HeatingPlate::HeatingPlate(int controlPin, ActuatorController::ControlMode mode, const char* name)
    : _controlPin(controlPin), _mode(mode), _name(name), status(false),
      _cycleTime(DEFAULT_CYCLE_TIME), lastCycleStart(0), dutyCycle(0) {
    pinMode(_controlPin, OUTPUT);
}

void HeatingPlate::begin() {
    digitalWrite(_controlPin, LOW);
    Logger::log(LogLevel::INFO, String(_name) + " initialized");
}

void HeatingPlate::control(bool state, int value) {
    if (state) {
        if (_mode == ActuatorController::ControlMode::Relay) {
            controlWithCycle(value / 100.0);
        } else {
            controlPWM(value);
        }
    } else {
        digitalWrite(_controlPin, LOW);
    }
    status = state;
    Logger::log(LogLevel::INFO, String(_name) + " control called with state: " + String(state) + " and value: " + String(value));
}

void HeatingPlate::controlRelay(bool state) {
    digitalWrite(_controlPin, state ? HIGH : LOW);
    status = state;
    Logger::log(LogLevel::INFO, String(_name) + (status ? " is ON" : " is OFF"));
}

void HeatingPlate::controlPWM(int value) {
    int pwmValue = map(constrain(value, 0, MAX_POWER_PERCENT), 0, MAX_POWER_PERCENT, 0, 255);
    analogWrite(_controlPin, pwmValue);
    status = (pwmValue > 0);
    Logger::log(LogLevel::INFO, String(_name) + (status ? " is ON with power: " + String(value) + "%" : " is OFF"));
}

bool HeatingPlate::isOn() const {
    return status;
}

void HeatingPlate::controlWithPID(double pidOutput) {
    int percentPower = map(pidOutput, 0, 100, 0, MAX_POWER_PERCENT);
    if (_mode == HeatingControlMode::RELAY) {
        controlWithCycle(percentPower / 100.0);
    } else {
        controlPWM(percentPower);
    }
    Logger::log(LogLevel::INFO, String(_name) + " PID control with power: " + String(percentPower) + "%");
}

void HeatingPlate::controlWithCycle(double percentage) {
    dutyCycle = percentage;
    unsigned long now = millis();
    if (now - lastCycleStart >= _cycleTime) {  
        lastCycleStart = now;
    }
    digitalWrite(_controlPin, (now - lastCycleStart < _cycleTime * dutyCycle) ? HIGH : LOW);  
    status = (dutyCycle > 0);
}

void HeatingPlate::setCycleTime(unsigned long newCycleTime) {
    _cycleTime = newCycleTime;
}
*/

#include "HeatingPlate.h"
#include "Logger.h"

HeatingPlate::HeatingPlate(int relayPin, bool isPWMCapable, const char* name)
    : _relayPin(relayPin), _name(name), _status(false), _isPWMCapable(isPWMCapable), _currentValue(0) {
    pinMode(_relayPin, OUTPUT);
}

void HeatingPlate::begin() {
    digitalWrite(_relayPin, HIGH);
    Logger::log(LogLevel::INFO, String(_name) + " initialized");
}

void HeatingPlate::control(bool state, int value) {
    value = constrain(value, 0, 100);

    if (state != _status || (state && value != _currentValue)) {
        if (state) {
            if (_isPWMCapable) {
                controlPWM(value);
            } else {
                controlWithCycle(value);
            }
        } else {
            controlOnOff(false);
        }
        _status = state;
        _currentValue = value;
    }
}

bool HeatingPlate::isOn() const {
    return _status;
}

void HeatingPlate::controlPWM(int value) {
    int pwmValue = map(value, 0, 100, 0, 255);
    analogWrite(_relayPin, pwmValue);
    _status = (pwmValue > 0);
    Logger::log(LogLevel::INFO, String(_name) + (_status ? " is ON with power: " + String(value) + "%" : " is OFF"));
}

void HeatingPlate::controlOnOff(bool state) {
    digitalWrite(_relayPin, state ? LOW : HIGH);
    _status = state;
    //Logger::log(LogLevel::INFO, String(_name) + (_status ? " is ON" : " is OFF"));
    Logger::log(LogLevel::INFO, String(_name) + (_status ? F(" is ON") : F(" is OFF")));
}

// This function implements a simple duty cycle control for the heating plate.
// It's used when the heating plate doesn't support PWM and only has on/off control.
// The function creates a PWM-like effect by turning the heater on and off
// over a fixed time period (cycle time) based on the desired power percentage.
void HeatingPlate::controlWithCycle(int percentage) {
    _dutyCycle = percentage / 100.0;  // Convert percentage to a decimal (0.0 to 1.0)
    unsigned long now = millis();
    
    // Check if we've completed a full cycle
    if (now - _lastCycleStart >= _cycleTime) {
        _lastCycleStart = now;  // Reset the cycle start time
    }
    
    // Determine if the heater should be on or off based on the current point in the cycle
    if (now - _lastCycleStart < _cycleTime * _dutyCycle) {
        digitalWrite(_relayPin, LOW);  // Turn heater ON
        _status = true;
    } else {
        digitalWrite(_relayPin, HIGH);   // Turn heater OFF
        _status = false;
    }
    
    Logger::log(LogLevel::INFO, String(_name) + " Duty Cycle: " + String(_dutyCycle * 100) + "%");
}