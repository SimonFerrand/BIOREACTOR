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
      pauseStartTime(0),
      isPIDEnabled(false) //PID activated by default ; false/true
{
}

void FermentationProgram::configure(float tempSetpoint, float phSetpoint, float doSetpoint,
                                    float nutrientConc, float baseConc, int duration,
                                    const String& experimentName, const String& comment) {  // &nutrientFixedFlowRate
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
    if (volumeManager.getCurrentVolume() == 0) {
      //Logger::log(LogLevel::ERROR, "Initial volume not set. Please set initial volume before starting fermentation.");
      Logger::log(LogLevel::ERROR, F("Initial volume not set. Please set initial volume before starting fermentation."));
      return;
    }
    _isRunning = true;
    _isPaused = false;
    startTime = millis();
    totalPauseTime = 0;

    nutrientFixedFlowRate = 100.0; // ml/min

    initializeStirringSpeed();
    pidManager.startTemperaturePID(tempSetpoint);
    pidManager.startPHPID(phSetpoint);
    pidManager.startDOPID(doSetpoint);


    //ActuatorController::runActuator("airPump", 50, 0);  // Start air pump at 50% speed
    //ActuatorController::runActuator("stirringMotor", 390, 0);  // Start stirring at 390 RPM

    /*
    Logger::log(LogLevel::INFO, "Fermentation started: " + experimentName);
    Logger::log(LogLevel::INFO, "Comment: " + comment);
    Logger::log(LogLevel::INFO, "Fermentation parameters:");
    Logger::log(LogLevel::INFO, "  Duration: " + String(duration) + " seconds");
    Logger::log(LogLevel::INFO, "  Initial volume: " + String(volumeManager.getCurrentVolume()) + " L");
    Logger::log(LogLevel::INFO, "  Max allowed volume: " + String(volumeManager.getMaxAllowedVolume()) + " L");
    Logger::log(LogLevel::INFO, "  Min volume: " + String(volumeManager.getMinVolume()) + " L");
    Logger::log(LogLevel::INFO, "Duration set to: " + String(duration) + " seconds");
    */

    Logger::log(LogLevel::INFO, String(F("Fermentation started: ")) + experimentName);
    Logger::log(LogLevel::INFO, String(F("Comment: ")) + comment);
    Logger::log(LogLevel::INFO, F("Fermentation parameters:"));
    Logger::log(LogLevel::INFO, String(F("  Duration: ")) + String(duration) + F(" seconds"));
    Logger::log(LogLevel::INFO, String(F("  Initial volume: ")) + String(volumeManager.getCurrentVolume()) + F(" L"));
    Logger::log(LogLevel::INFO, String(F("  Max allowed volume: ")) + String(volumeManager.getMaxAllowedVolume()) + F(" L"));
    Logger::log(LogLevel::INFO, String(F("  Min volume: ")) + String(volumeManager.getMinVolume()) + F(" L"));
    Logger::log(LogLevel::INFO, String(F("Duration set to: ")) + String(duration) + F(" seconds"));

}

void FermentationProgram::update() {
    if (!_isRunning || _isPaused) return;

    if (isPIDEnabled) {
        pidManager.updateAllPIDControllers();
        pidManager.adjustPIDStirringSpeed();
    } else {
        // Make sure the stirring motor is running at minimum speed
        ActuatorController::runActuator("stirringMotor", currentStirringSpeed, 0);
    }

    updateVolume();
    addNutrientsContinuouslyFixedRate(nutrientFixedFlowRate);
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

    //Logger::log(LogLevel::INFO, "Fermentation paused");
    Logger::log(LogLevel::INFO, F("Fermentation paused"));
}

void FermentationProgram::resume() {
    if (!_isRunning || !_isPaused) return;

    _isPaused = false;
    totalPauseTime += millis() - pauseStartTime;

    pidManager.resumeAllPID();

    ActuatorController::runActuator("airPump", 50, 0);  // Resume air pump at 50% speed

    //Logger::log(LogLevel::INFO, "Fermentation resumed");
    Logger::log(LogLevel::INFO, F("Fermentation resumed"));
}

void FermentationProgram::stop() {
    if (!_isRunning) return;

    _isRunning = false;
    _isPaused = false;

    ActuatorController::stopAllActuators();
    pidManager.stop();

    //Logger::log(LogLevel::INFO, "Fermentation stopped");
    Logger::log(LogLevel::INFO, F("Fermentation stopped"));
}

void FermentationProgram::updateVolume() {
    volumeManager.updateVolume();
}

/*
void FermentationProgram::checkCompletion() {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - startTime - totalPauseTime;
    if (elapsedTime >= static_cast<unsigned long>(duration) * 1000UL) {
        stop();
        Logger::log(LogLevel::INFO, "Fermentation completed");
    }
}
*/

void FermentationProgram::checkCompletion() {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - startTime - totalPauseTime;
    
    // Vérifiez qu'au moins 1 seconde s'est écoulée avant de faire la vérification
    if (elapsedTime < 1000) return;

    if (elapsedTime >= static_cast<unsigned long>(duration) * 1000UL) {
        stop();
        Logger::log(LogLevel::INFO, "Fermentation completed. Elapsed time: " + String(elapsedTime/1000) + " s");
    }
}

void FermentationProgram::initializeStirringSpeed() {
    int minSpeed = max(MIN_STIRRING_SPEED, ActuatorController::getStirringMotorMinRPM());
    currentStirringSpeed = min(minSpeed, ActuatorController::getStirringMotorMaxRPM());
    pidManager.setMinStirringSpeed(currentStirringSpeed);
    ActuatorController::runActuator("stirringMotor", currentStirringSpeed, 0);  // 0 for continuous operation
    Logger::log(LogLevel::INFO, "Fermentation stirring speed initialized to: " + String(currentStirringSpeed) + " RPM");
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
    // Calculate elapsed time in hours
    float elapsedTime = (millis() - startTime) / 3600000.0;
    // Calculate total amount of nutrients to add based on current volume and desired concentration
    float totalNutrientToAdd = nutrientConc * volumeManager.getCurrentVolume();
    // Calculate the amount of nutrients to add now based on elapsed time
    float nutrientToAddNow = (totalNutrientToAdd / duration) * elapsedTime;
    // Check if it's safe to add the calculated amount of nutrients
    if (volumeManager.isSafeToAddVolume(nutrientToAddNow)) {
        // Get the maximum flow rate of the nutrient pump
        float maxFlowRate = ActuatorController::getPumpMaxFlowRate("nutrientPump");
        // Calculate how long the pump should run to add the desired amount of nutrients
        float pumpDuration = (nutrientToAddNow / maxFlowRate) * 60000; // Convert to milliseconds
        // Activate the nutrient pump
        ActuatorController::runActuator("nutrientPump", maxFlowRate, pumpDuration);
        // Record the volume change
        volumeManager.recordVolumeChange(nutrientToAddNow, "Nutrient");
        Logger::log(LogLevel::INFO, "Added nutrients: " + String(nutrientToAddNow) + " ml");
    } else {
        //Logger::log(LogLevel::WARNING, "Cannot add nutrients: Volume limit reached");
        Logger::log(LogLevel::WARNING, F("Cannot add nutrients: Volume limit reached"));
    }
}

void FermentationProgram::addNutrientsContinuouslyFixedRate(float fixedFlowRate) {
    unsigned long currentTime = millis();
    if (isAddingNutrients) {
        // Check if it's time to stop adding nutrients
        if (currentTime - lastNutrientActivationTime >= NUTRIENT_ACTIVATION_TIME) {
            ActuatorController::stopActuator("nutrientPump");
            isAddingNutrients = false;
            lastNutrientActivationTime = currentTime;
            //Logger::log(LogLevel::INFO, "Stopped adding nutrients");
            Logger::log(LogLevel::INFO, F("Stopped adding nutrients"));
        }
        return;
    }
    // Check if we're still in the pause period
    if (currentTime - lastNutrientActivationTime < (NUTRIENT_ACTIVATION_TIME + NUTRIENT_PAUSE_TIME)) {
        return;
    }
    // Check if fermentation is running
    if (!_isRunning || _isPaused) {
        return;
    }
    // Get current volume and maximum allowed volume
    float currentVolume = volumeManager.getCurrentVolume();
    float maxAllowedVolume = volumeManager.getMaxAllowedVolume();
    // Check if volume limit is reached or duration is exceeded
    if (currentVolume >= maxAllowedVolume || (currentTime - startTime) > (duration * 1000UL)) {
        stop();
        //Logger::log(LogLevel::INFO, "Fermentation stopped: Volume limit reached or duration exceeded");
        Logger::log(LogLevel::INFO, F("Fermentation stopped: Volume limit reached or duration exceeded"));
        return;
    }
    // Calculate the amount of nutrient to add
    float availableVolume = maxAllowedVolume - currentVolume;
    float nutrientToAdd = min((fixedFlowRate * NUTRIENT_ACTIVATION_TIME / 60000.0), availableVolume);
    // Start adding nutrients if there's room
    if (nutrientToAdd > 0) {
        ActuatorController::runActuator("nutrientPump", fixedFlowRate, NUTRIENT_ACTIVATION_TIME);
        volumeManager.recordVolumeChange(nutrientToAdd, "Nutrient");
        Logger::log(LogLevel::INFO, "Started adding nutrients: " + String(nutrientToAdd) + " ml");
        isAddingNutrients = true;
        lastNutrientActivationTime = currentTime;
    }
}