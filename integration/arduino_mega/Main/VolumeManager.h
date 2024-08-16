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

    float getAddedNaOH() const { return addedNaOH; }
    float getAddedNutrient() const { return addedNutrient; }
    float getAddedMicroalgae() const { return addedMicroalgae; }
    float getRemovedVolume() const { return removedVolume; }
    
    float getTotalVolume() const { return totalVolume; }
    float getMaxAllowedVolume() const { return totalVolume * maxVolumePercent; }
    bool isSafeToAddVolume(float volume) const;

private:
    float totalVolume;
    float maxVolumePercent;
    float minVolume;
    float currentVolume;
    float addedNaOH;
    float addedNutrient;
    float addedMicroalgae;
    float removedVolume;

    const float MAX_VOLUME_PERCENT = 0.95; // 95% of total volume
    const float SAFE_ADDITION_PERCENT = 0.05; // 5% of available volume
};

#endif // VOLUME_MANAGER_H