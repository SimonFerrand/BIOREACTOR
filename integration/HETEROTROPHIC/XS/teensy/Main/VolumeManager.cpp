#include "VolumeManager.h"
#include "ActuatorController.h"
#include "Logger.h"

VolumeManager::VolumeManager(float totalVolume, float maxVolumePercent, float minVolume)
    : totalVolume(totalVolume), maxVolumePercent(maxVolumePercent), minVolume(minVolume),
      currentVolume(0), addedNaOH(0), addedNutrient(0), addedMicroalgae(0), removedVolume(0),
      cumulativeNaOH(0), cumulativeNutrient(0), cumulativeMicroalgae(0), cumulativeRemoved(0) {}

void VolumeManager::updateVolume() {
    updateVolumeFromActuators();
    float volumeChange = addedNaOH + addedNutrient + addedMicroalgae - removedVolume;
    if (abs(volumeChange) > 0.0001) { // Ignore very small changes
        currentVolume += volumeChange;
        currentVolume = max(minVolume, min(currentVolume, totalVolume * maxVolumePercent));
        // Update cumulative totals
        cumulativeNaOH  += addedNaOH;
        cumulativeNutrient  += addedNutrient;
        cumulativeMicroalgae  += addedMicroalgae;
        cumulativeRemoved  += removedVolume;
        //Logger::log(LogLevel::INFO, "Volumes cumulatifs mis à jour - NaOH: " + String(cumulativeNaOH, 6) + "L, Nutrient: " + String(cumulativeNutrient, 6) + "L");       
    }
    // Reset temporary counters
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
        initialVolume = volume;
        currentVolume = initialVolume;
        addedNaOH = addedNutrient = addedMicroalgae = removedVolume = 0;
        cumulativeNaOH = cumulativeNutrient = cumulativeMicroalgae = cumulativeRemoved = 0;
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
/*
void VolumeManager::recordVolumeChange(float volume, const String& source) {
    if (source == "NaOH") addedNaOH += volume;
    else if (source == "Nutrient") addedNutrient += volume;
    else if (source == "Microalgae") addedMicroalgae += volume;
    else if (source == "Removed") removedVolume += volume;
    Logger::log(LogLevel::INFO, "Volume change recorded: " + String(volume, 6) + 
            " L from " + source + ", Total change: " + 
            String(addedNaOH + addedNutrient + addedMicroalgae - removedVolume, 6) + " L");
}
*/

void VolumeManager::recordVolumeChange(float volume, const String& source) {
    // Convertir la source en majuscules pour la comparaison
    String sourceUpper = source;
    sourceUpper.toUpperCase();

    if (sourceUpper == "NAOH") {
        addedNaOH += volume;
    }
    else if (sourceUpper == "NUTRIENT") {
        addedNutrient += volume;
    }
    else if (sourceUpper == "MICROALGAE") {
        addedMicroalgae += volume;
    }
    else if (sourceUpper == "REMOVED") {
        removedVolume += volume;
    }

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
    info += "Current Volume: " + String(getCurrentVolume(), 4) + " L\n";
    info += "Total Volume: " + String(getTotalVolume(), 4) + " L\n";
    info += "Max Allowed Volume: " + String(getMaxAllowedVolume(), 4) + " L\n";
    info += "Min Volume: " + String(getMinVolume(), 4) + " L\n";
    info += "Available Volume: " + String(getAvailableVolume()) + " L\n";
    info += "Added NaOH: " + String(getAddedNaOH(), 4) + " L\n";
    info += "Added Nutrient: " + String(getAddedNutrient(), 4) + " L\n";
    info += "Added Microalgae: " + String(getAddedMicroalgae(), 4) + " L\n";
    info += "Removed Volume: " + String(getRemovedVolume(), 4) + " L\n";
    return info;
}

void VolumeManager::resetVolume() {
        currentVolume = initialVolume;
        addedNaOH = 0;
        addedNutrient = 0;
        addedMicroalgae = 0;
        removedVolume = 0;
        cumulativeNaOH = 0;
        cumulativeNutrient = 0;
        cumulativeMicroalgae = 0;
        cumulativeRemoved = 0;
        Logger::log(LogLevel::INFO, "Volume reset to initial conditions: " + String(initialVolume) + " L");
    }