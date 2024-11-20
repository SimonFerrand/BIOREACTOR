// DrainProgram.cpp
#include "DrainProgram.h"
#include <Arduino.h>
#include "Logger.h"
#include "ProgramBase.h"

DrainProgram::DrainProgram() : rate(0), duration(0), startTime(0) {}

void DrainProgram::start(const String& command) {
    // Parse command: "drain <rate> <duration>"
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);
    if (firstSpace != -1 && secondSpace != -1) {
        rate = command.substring(firstSpace + 1, secondSpace).toInt();
        duration = command.substring(secondSpace + 1).toInt();
        
        _isRunning = true;
        _isPaused = false;
        startTime = millis();
        
        ActuatorController::runActuator("drainPump", rate, 0); // 0 for continuous operation
        Logger::log(LogLevel::INFO, "Drain started at rate: " + String(rate));
    } else {
        //Logger::log(LogLevel::ERROR, "Invalid drain command format");
        Logger::log(LogLevel::ERROR, F("Invalid drain command format"));
    }
}

void DrainProgram::update() {
    if (!_isRunning || _isPaused) return;

    if (millis() - startTime >= duration * 1000UL) {
        stop();
        //Logger::log(LogLevel::INFO, "Drain finished");
        Logger::log(LogLevel::INFO, F("Drain finished"));
    }
}

void DrainProgram::pause() {
    if (_isRunning && !_isPaused) {
        ActuatorController::stopActuator("drainPump");
        _isPaused = true;
        //Logger::log(LogLevel::INFO, "Drain paused");
        Logger::log(LogLevel::INFO, F("Drain paused"));
    }
}

void DrainProgram::resume() {
    if (_isRunning && _isPaused) {
        ActuatorController::runActuator("drainPump", rate, 0);
        _isPaused = false;
        //Logger::log(LogLevel::INFO, "Drain resumed");
        Logger::log(LogLevel::INFO, F("Drain resumed"));
    }
}

void DrainProgram::stop() {
    if (_isRunning) {
        ActuatorController::stopActuator("drainPump");
        _isRunning = false;
        _isPaused = false;
    }
}

void DrainProgram::parseCommand(const String& command) {
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);
    if (firstSpace != -1 && secondSpace != -1) {
        rate = command.substring(firstSpace + 1, secondSpace).toInt();
        duration = command.substring(secondSpace + 1).toInt();
    } else {
        //Logger::log(LogLevel::ERROR, "Invalid drain command format");
        Logger::log(LogLevel::ERROR, F("Invalid drain command format"));
    }
}

void DrainProgram ::getParameters(JsonDocument& doc) const {
      doc["rate"] = rate;
      doc["dur"] = duration;
}