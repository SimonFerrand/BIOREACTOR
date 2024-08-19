// MixProgram.cpp
#include "MixProgram.h"
#include <Arduino.h>

MixProgram::MixProgram() : speed(0) {}

void MixProgram::start(const String& command) {
    // Parse command: "mix <speed>"
    int spaceIndex = command.indexOf(' ');
    if (spaceIndex != -1) {
        speed = command.substring(spaceIndex + 1).toInt();
        
        _isRunning = true;
        _isPaused = false;
        
        ActuatorController::runActuator("stirringMotor", speed, 0); // 0 for continuous operation
        Logger::log(LogLevel::INFO, "Mixing started at speed: " + String(speed));
    } else {
        //Logger::log(LogLevel::ERROR, "Invalid mix command format");
        Logger::log(LogLevel::ERROR, F("Invalid mix command format"));
    }
}

void MixProgram::update() {
    // The mixing process continues until it's stopped
    // No additional logic needed here
}

void MixProgram::pause() {
    if (_isRunning && !_isPaused) {
        ActuatorController::stopActuator("stirringMotor");
        _isPaused = true;
        //Logger::log(LogLevel::INFO, "Mixing paused");
        Logger::log(LogLevel::INFO, F("Mixing paused"));
    }
}

void MixProgram::resume() {
    if (_isRunning && _isPaused) {
        ActuatorController::runActuator("stirringMotor", speed, 0);
        _isPaused = false;
        //Logger::log(LogLevel::INFO, "Mixing resumed");
        Logger::log(LogLevel::INFO, F("Mixing resumed"));
    }
}

void MixProgram::stop() {
    if (_isRunning) {
        ActuatorController::stopActuator("stirringMotor");
        _isRunning = false;
        _isPaused = false;
        //Logger::log(LogLevel::INFO, "Mixing stopped");
        Logger::log(LogLevel::INFO, F("Mixing stopped"));
    }
}

void MixProgram::parseCommand(const String& command) {
    int spaceIndex = command.indexOf(' ');
    if (spaceIndex != -1) {
        speed = command.substring(spaceIndex + 1).toInt();
    } else {
        //Logger::log(LogLevel::ERROR, "Invalid mix command format");
        Logger::log(LogLevel::ERROR, F("Invalid mix command format"));
    }
}
