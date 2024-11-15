#ifndef VOLUME_MANAGER_H
#define VOLUME_MANAGER_H

#include <Arduino.h>

class VolumeManager {
public:
    VolumeManager(float totalVolume, float maxVolumePercent, float minVolume);

    void updateVolume();
    void manuallyAdjustVolume(float volume, const String& source);
    float getCurrentVolume() const { return currentVolume; }
    void setInitialVolume(float volume);
    float getAvailableVolume() const;
    float getMaxSafeAddition() const;
    void recordVolumeChange(float volume, const String& source);
    void updateVolumeFromActuators();

    float getAddedNaOH() const { return cumulativeNaOH; }
    float getAddedNutrient() const { return cumulativeNutrient; }
    float getAddedMicroalgae() const { return cumulativeMicroalgae; }
    float getRemovedVolume() const { return cumulativeRemoved; }
    
    float getTotalVolume() const { return totalVolume; }
    float getMinVolume() const { return minVolume; }
    float getMaxAllowedVolume() const { return totalVolume * maxVolumePercent; }
    bool isSafeToAddVolume(float volume) const;
    String getVolumeInfo() const;
    void resetVolume();

private:
    float totalVolume;
    float maxVolumePercent;
    float minVolume;
    float currentVolume;
    float initialVolume;

    // Temporary volumes for current changes
    float addedNaOH;
    float addedNutrient;
    float addedMicroalgae;
    float removedVolume;
    
    // Total cumulative volumes
    float cumulativeNaOH;
    float cumulativeNutrient;
    float cumulativeMicroalgae;
    float cumulativeRemoved;

    const float MAX_VOLUME_PERCENT = 0.95; // 95% of total volume
    const float SAFE_ADDITION_PERCENT = 0.05; // 5% of available volume
};

#endif // VOLUME_MANAGER_H