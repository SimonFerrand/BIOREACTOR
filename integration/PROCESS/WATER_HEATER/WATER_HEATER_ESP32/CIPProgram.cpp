// CIPProgram.cpp
#include "CIPProgram.h"

CIPProgram::CIPProgram(PIDManager& pidManager) 
   : _pidManager(pidManager),
   targetTemp(0),
   duration(0),
   startTime(0)
{
}

void CIPProgram::start(const String& command) {
   parseCommand(command);
   if(targetTemp > 0 && duration > 0) {
       _isRunning = true;
       _isPaused = false;
       startTime = millis();
       _pidManager.startTemperaturePID(targetTemp);
       Logger::log(Logger::LogLevel::INFO, "CIP Started - Target: " + String(targetTemp) + "°C, Duration: " + String(duration) + " min"); 
   }
}

void CIPProgram::update() {
   if (!_isRunning || _isPaused) return;

    static unsigned long lastPIDUpdate = 0;
    unsigned long currentTime = millis();
    // Mettre à jour le PID toutes les X millisecondes
    if (currentTime - lastPIDUpdate >= PID_UPDATE_TEMP) {
        _pidManager.updateAllPIDControllers();
        lastPIDUpdate = currentTime;
    }
   
   if (millis() - startTime >= duration * 60000UL) {
       stop();
       Logger::log(Logger::LogLevel::INFO, F("CIP completed"));
   }
}

void CIPProgram::stop() {
   if (_isRunning) {
       _pidManager.stopTemperaturePID();
       _isRunning = false;
       _isPaused = false;
       Logger::log(Logger::LogLevel::INFO, F("CIP stopped"));
   }
}

void CIPProgram::pause() {
   if (_isRunning && !_isPaused) {
       _pidManager.stopTemperaturePID();
       _isPaused = true;
       Logger::log(Logger::LogLevel::INFO, F("CIP paused"));
   }
}

void CIPProgram::resume() {
   if (_isRunning && _isPaused) {
       _pidManager.startTemperaturePID(targetTemp);
       _isPaused = false;
       Logger::log(Logger::LogLevel::INFO, F("CIP resumed"));
   }
}

void CIPProgram::parseCommand(const String& command) {
   int firstSpace = command.indexOf(' ');
   int secondSpace = command.indexOf(' ', firstSpace + 1);
   if (firstSpace != -1 && secondSpace != -1) {
       targetTemp = command.substring(firstSpace + 1, secondSpace).toFloat();
       duration = command.substring(secondSpace + 1).toInt();
   } else {
       Logger::log(Logger::LogLevel::ERROR, F("Invalid CIP command format"));
   }
}

void CIPProgram::getParameters(JsonDocument& doc) const {
   doc["target_temp"] = targetTemp;
   doc["duration"] = duration; 
   doc["elapsed_time"] = (millis() - startTime) / 60000UL;
}