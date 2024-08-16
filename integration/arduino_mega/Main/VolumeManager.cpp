#include "VolumeManager.h"
#include "ActuatorController.h"
#include "Logger.h"

VolumeManager::VolumeManager(float totalVolume, float maxVolumePercent, float minVolume)
    : totalVolume(totalVolume), maxVolumePercent(maxVolumePercent), minVolume(minVolume),
      currentVolume(minVolume), addedNaOH(0), addedNutrient(0), addedMicroalgae(0), removedVolume(0) {}

void VolumeManager::updateVolume() {
    updateVolumeFromActuators();
    currentVolume += (addedNaOH + addedNutrient + addedMicroalgae - removedVolume);
    currentVolume = max(minVolume, min(currentVolume, totalVolume * maxVolumePercent));

    addedNaOH = 0;
    addedNutrient = 0;
    addedMicroalgae = 0;
    removedVolume = 0;

    Logger::log(LogLevel::INFO, "Current volume updated: " + String(currentVolume) + " L");
}

void VolumeManager::manuallyAdjustVolume(float volume, const String& source) {
    recordVolumeChange(volume, source);
    updateVolume();
    Logger::log(LogLevel::INFO, "Volume manually adjusted: " + String(volume) + " L from " + source);
}

void VolumeManager::setInitialVolume(float volume) {
    if (volume > 0 && volume <= totalVolume * maxVolumePercent) {
        currentVolume = volume;
        addedNaOH = 0;
        addedNutrient = 0;
        addedMicroalgae = 0;
        removedVolume = 0;
        Logger::log(LogLevel::INFO, "Initial volume set to: " + String(volume) + " L");
    }
}

float VolumeManager::getAvailableVolume() const {
    return (totalVolume * maxVolumePercent) - currentVolume;
}

float VolumeManager::getMaxSafeAddition() const {
    return getAvailableVolume() * SAFE_ADDITION_PERCENT;
}

void VolumeManager::recordVolumeChange(float volume, const String& source) {
    if (source == "NaOH") addedNaOH += volume;
    else if (source == "Nutrient") addedNutrient += volume;
    else if (source == "Microalgae") addedMicroalgae += volume;
    else if (source == "Removed") removedVolume += volume;
}

void VolumeManager::updateVolumeFromActuators() {
    float nutrientPumpVolume = ActuatorController::getVolumeAdded("nutrientPump");
    float basePumpVolume = ActuatorController::getVolumeAdded("basePump");
    float drainPumpVolume = ActuatorController::getVolumeRemoved("drainPump");

    addedNutrient += nutrientPumpVolume;
    addedNaOH += basePumpVolume;
    removedVolume += drainPumpVolume;

    ActuatorController::resetVolumeAdded("nutrientPump");
    ActuatorController::resetVolumeAdded("basePump");
    ActuatorController::resetVolumeRemoved("drainPump");
}

bool VolumeManager::isSafeToAddVolume(float volume) const {
    return (currentVolume + volume) <= getMaxAllowedVolume();
}