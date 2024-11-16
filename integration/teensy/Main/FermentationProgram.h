#ifndef FERMENTATION_PROGRAM_H
#define FERMENTATION_PROGRAM_H

#include "ProgramBase.h"
#include "PIDManager.h"
#include "VolumeManager.h"
#include "ActuatorController.h"
#include "SensorController.h"
#include "Logger.h"

class FermentationProgram : public ProgramBase {
public:
    FermentationProgram(PIDManager& pidManager, VolumeManager& volumeManager);
    void configure(float tempSetpoint, float phSetpoint, float doSetpoint,
                   float nutrientConc, float baseConc, float durationHours, float nutrientDelayHours,
                   const String& experimentName, const String& comment);

    void start(const String& command) override;
    void update() override;
    void pause() override;
    void resume() override;
    void stop() override;
    bool isRunning() const override { return _isRunning; }
    bool isPaused() const override { return _isPaused; }
    String getName() const override { return "Fermentation"; }
    void parseCommand(const String& command) override;
    void initializeStirringSpeed();
    void setNutrientFixedFlowRate(float rate) { nutrientFixedFlowRate = rate; }
    
    // Time settings for continuous nutrient addition (in milliseconds)
    static const unsigned long NUTRIENT_ACTIVATION_TIME = 10000; //60000;  // 1 minute   10000
    static const unsigned long NUTRIENT_PAUSE_TIME = 508400; // 8.64 minutes - 10 secondes = 508.4 secondes
    static constexpr float DEFAULT_NUTRIENT_FLOW_RATE = 3.0; // ml/min

    void setPIDEnabled(bool enabled);
    //void setPIDEnabled(bool enabled) { isPIDEnabled = enabled; }

    float getTempSetpoint() const { return tempSetpoint; }
    float getPHSetpoint() const { return phSetpoint; }
    float getDOSetpoint() const { return doSetpoint; }
    float getNutrientConc() const { return nutrientConc; }
    float getBaseConc() const { return baseConc; }
    float getDuration() const { return duration / 3600000.0; } // Convert milliseconds into hours
    String getExperimentName() const { return experimentName; }
    String getComment() const { return comment; }
    
    void getParameters(JsonDocument& doc) const override;

    void updateTurbidity();

    void setNutrientStartDelay(float delayHours) { nutrientStartDelay = static_cast<unsigned long>(delayHours * 3600000.0); }
    float getNutrientStartDelay() const { return nutrientStartDelay / 3600000.0; } // Convertit en heures

private:
    PIDManager& pidManager;
    VolumeManager& volumeManager;
    float tempSetpoint;
    float phSetpoint;
    float doSetpoint;
    float nutrientConc;
    float baseConc;
    unsigned long duration;
    String experimentName;
    String comment;

    unsigned long startTime;
    unsigned long pauseStartTime;
    unsigned long totalPauseTime;

    int currentStirringSpeed;

    unsigned long lastNutrientActivationTime;

    bool isPIDEnabled;

    void updateVolume();
    void checkCompletion();

    void addNutrientsContinuously();
    void addNutrientsContinuouslyFixedRate(float fixedFlowRate);
    
    bool isAddingNutrients;
    bool isAnyActuatorRunning() const;
    unsigned long plannedNutrientActivationTime;

    static const int MIN_STIRRING_SPEED = 390;
    float nutrientFixedFlowRate = DEFAULT_NUTRIENT_FLOW_RATE;

    unsigned long nutrientStartDelay = 0;
    bool nutrientAdditionStarted = false;

};

#endif // FERMENTATION_PROGRAM_H