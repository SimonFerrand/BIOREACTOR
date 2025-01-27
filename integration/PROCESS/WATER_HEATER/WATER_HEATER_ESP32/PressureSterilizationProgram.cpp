// PressureSterilizationProgram.cpp
#include "PressureSterilizationProgram.h"

PressureSterilizationProgram::PressureSterilizationProgram(PIDManager& pidManager) 
    : _pidManager(pidManager)
{
}

void PressureSterilizationProgram::start(const String& command) {
    _isRunning = true;
    parseCommand(command);
}

void PressureSterilizationProgram::update() {
    if (!_isRunning) return;

}

void PressureSterilizationProgram::stop() {
    _isRunning = false;
    _pidManager.stop();
}

void PressureSterilizationProgram::pause() {
    _isPaused = true;
}

void PressureSterilizationProgram::resume() {
    _isPaused = false;
}

void PressureSterilizationProgram::parseCommand(const String& command) {

}

void PressureSterilizationProgram::getParameters(JsonDocument& doc) const {
    doc["status"] = _isRunning ? "running" : "stopped";
    doc["paused"] = _isPaused;
}