#include "FermentationProgram.h"
#include <Arduino.h>
#include "Logger.h"

FermentationProgram::FermentationProgram(PIDManager& pidManager, VolumeManager& volumeManager)
    : ProgramBase(),
      pidManager(pidManager),
      volumeManager(volumeManager),
      totalPauseTime(0),
      tempSetpoint(0),
      phSetpoint(0),
      doSetpoint(0),
      nutrientConc(0),
      baseConc(0),
      duration(0),
      startTime(0),
      pauseStartTime(0)
{
}

void FermentationProgram::configure(float tempSetpoint, float phSetpoint, float doSetpoint,
                                    float nutrientConc, float baseConc, int duration,
                                    const String& experimentName, const String& comment) {
    this->tempSetpoint = tempSetpoint;
    this->phSetpoint = phSetpoint;
    this->doSetpoint = doSetpoint;
    this->nutrientConc = nutrientConc;
    this->baseConc = baseConc;
    this->duration = duration;
    this->experimentName = experimentName;
    this->comment = comment;

    pidManager.setTemperatureSetpoint(tempSetpoint);
    pidManager.setPHSetpoint(phSetpoint);
    pidManager.setDOSetpoint(doSetpoint);
}

void FermentationProgram::start(const String& command) {
    _isRunning = true;
    _isPaused = false;
    startTime = millis();
    totalPauseTime = 0;

    initializeStirringSpeed();
    pidManager.startTemperaturePID(tempSetpoint);
    pidManager.startPHPID(phSetpoint);
    pidManager.startDOPID(doSetpoint);

    //ActuatorController::runActuator("airPump", 50, 0);  // Start air pump at 50% speed
    //ActuatorController::runActuator("stirringMotor", 390, 0);  // Start stirring at 390 RPM

    Logger::log(LogLevel::INFO, "Fermentation started: " + experimentName);
    Logger::log(LogLevel::INFO, "Comment: " + comment);
}

void FermentationProgram::update() {
    if (!_isRunning || _isPaused) return;

    pidManager.updateAllPIDControllers();
    pidManager.adjustPIDStirringSpeed();
    updateVolume();
    checkCompletion();
}

void FermentationProgram::pause() {
    if (!_isRunning || _isPaused) return;

    _isPaused = true;
    pauseStartTime = millis();

    pidManager.pauseAllPID();

    ActuatorController::runActuator("stirringMotor", 390, 0);  // Minimum stirring speed
    ActuatorController::stopActuator("airPump");
    ActuatorController::stopActuator("nutrientPump");
    ActuatorController::stopActuator("basePump");

    Logger::log(LogLevel::INFO, "Fermentation paused");
}

void FermentationProgram::resume() {
    if (!_isRunning || !_isPaused) return;

    _isPaused = false;
    totalPauseTime += millis() - pauseStartTime;

    pidManager.resumeAllPID();

    ActuatorController::runActuator("airPump", 50, 0);  // Resume air pump at 50% speed

    Logger::log(LogLevel::INFO, "Fermentation resumed");
}

void FermentationProgram::stop() {
    if (!_isRunning) return;

    _isRunning = false;
    _isPaused = false;

    ActuatorController::stopAllActuators();
    pidManager.stop();

    Logger::log(LogLevel::INFO, "Fermentation stopped");
}

void FermentationProgram::updateVolume() {
    volumeManager.updateVolume();
}

void FermentationProgram::checkCompletion() {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - startTime - totalPauseTime;
    if (elapsedTime >= static_cast<unsigned long>(duration) * 1000UL) {
        stop();
        Logger::log(LogLevel::INFO, "Fermentation completed");
    }
}

void FermentationProgram::initializeStirringSpeed() {
    int minSpeed = max(MIN_STIRRING_SPEED, ActuatorController::getStirringMotorMinRPM());
    minSpeed = min(minSpeed, ActuatorController::getStirringMotorMaxRPM());
    pidManager.setMinStirringSpeed(minSpeed);
    Logger::log(LogLevel::INFO, "Fermentation min stirring speed set to: " + String(minSpeed) + " RPM");
}

void FermentationProgram::parseCommand(const String& command) {
    char expName[50], comm[50];
    sscanf(command.c_str(), "fermentation %f %f %f %f %f %d %s %s",
           &tempSetpoint, &phSetpoint, &doSetpoint, &nutrientConc, &baseConc, &duration,
           expName, comm);
    experimentName = String(expName);
    comment = String(comm);
}

void FermentationProgram::addNutrientsContinuously() {
    float elapsedTime = (millis() - startTime) / 3600000.0; // Temps écoulé en heures
    float totalNutrientToAdd = nutrientConc * volumeManager.getCurrentVolume();
    float nutrientToAddNow = (totalNutrientToAdd / duration) * elapsedTime;
    
    if (volumeManager.isSafeToAddVolume(nutrientToAddNow)) {
        float maxFlowRate = ActuatorController::getPumpMaxFlowRate("nutrientPump");
        float pumpDuration = (nutrientToAddNow / maxFlowRate) * 60000; // Convertir en millisecondes
        
        ActuatorController::runActuator("nutrientPump", maxFlowRate, pumpDuration);
        volumeManager.recordVolumeChange(nutrientToAddNow, "Nutrient");
        
        Logger::log(LogLevel::INFO, "Added nutrients: " + String(nutrientToAddNow) + " ml");
    } else {
        Logger::log(LogLevel::WARNING, "Cannot add nutrients: Volume limit reached");
    }
}