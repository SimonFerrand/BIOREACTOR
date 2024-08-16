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
    : _relayPin(relayPin), _name(name), _status(false), _isPWMCapable(isPWMCapable) {
    pinMode(_relayPin, OUTPUT);
}

void HeatingPlate::begin() {
    digitalWrite(_relayPin, LOW);
    Logger::log(LogLevel::INFO, String(_name) + " initialized");
}

void HeatingPlate::control(bool state, int value) {
    if (state) {
        if (_isPWMCapable) {
            controlPWM(value);
        } else {
            controlWithCycle(value);
        }
    } else {
        controlOnOff(false);
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
    digitalWrite(_relayPin, state ? HIGH : LOW);
    _status = state;
    Logger::log(LogLevel::INFO, String(_name) + (_status ? " is ON" : " is OFF"));
}

void HeatingPlate::controlWithCycle(int percentage) {
    _dutyCycle = percentage / 100.0;
    unsigned long now = millis();
    if (now - _lastCycleStart >= _cycleTime) {
        _lastCycleStart = now;
    }
    if (now - _lastCycleStart < _cycleTime * _dutyCycle) {
        digitalWrite(_relayPin, HIGH);
        _status = true;
    } else {
        digitalWrite(_relayPin, LOW);
        _status = false;
    }
    Logger::log(LogLevel::INFO, String(_name) + " Duty Cycle: " + String(_dutyCycle * 100) + "%");
}