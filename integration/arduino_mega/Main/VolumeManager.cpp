#include "VolumeManager.h"
#include "ActuatorController.h"
#include "Logger.h"

VolumeManager::VolumeManager(float totalVolume, float maxVolumePercent, float minVolume)
    : totalVolume(totalVolume), maxVolumePercent(maxVolumePercent), minVolume(minVolume),
      currentVolume(0), addedNaOH(0), addedNutrient(0), addedMicroalgae(0), removedVolume(0) {}

void VolumeManager::updateVolume() {
    updateVolumeFromActuators();
    float volumeChange = addedNaOH + addedNutrient + addedMicroalgae - removedVolume;
    if (abs(volumeChange) > 0.0001) { // Ignore very small changes
        currentVolume += volumeChange;
        currentVolume = max(minVolume, min(currentVolume, totalVolume * maxVolumePercent));
        //Logger::log(LogLevel::INFO, "Volume updated: Current=" + String(currentVolume, 4) + " L, Change=" + String(volumeChange, 4) + " L");
    }
    addedNaOH = 0;
    addedNutrient = 0;
    addedMicroalgae = 0;
    removedVolume = 0;       
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
    } else {
    Logger::log(LogLevel::WARNING, "Invalid initial volume: " + String(volume) + " L. Must be between " + 
        String(minVolume) + " L and " + String(totalVolume * maxVolumePercent) + " L. Volume not changed.");
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
    Logger::log(LogLevel::INFO, "Volume change recorded: " + String(volume, 6) + 
            " L from " + source + ", Total change: " + 
            String(addedNaOH + addedNutrient + addedMicroalgae - removedVolume, 6) + " L");
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
    return (currentVolume + volume) <= (totalVolume * maxVolumePercent);
}

String VolumeManager::getVolumeInfo() const {
  
    String info = "Volume Information:\n";
    info += "Current Volume: " + String(getCurrentVolume()) + " L\n";
    info += "Total Volume: " + String(getTotalVolume()) + " L\n";
    info += "Max Allowed Volume: " + String(getMaxAllowedVolume()) + " L\n";
    info += "Min Volume: " + String(getMinVolume()) + " L\n";
    info += "Available Volume: " + String(getAvailableVolume()) + " L\n";
    info += "Added NaOH: " + String(getAddedNaOH()) + " L\n";
    info += "Added Nutrient: " + String(getAddedNutrient()) + " L\n";
    info += "Added Microalgae: " + String(getAddedMicroalgae()) + " L\n";
    info += "Removed Volume: " + String(getRemovedVolume()) + " L\n";
    return info;
}