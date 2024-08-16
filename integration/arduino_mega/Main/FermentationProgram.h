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
                   float nutrientConc, float baseConc, int duration,
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
    void addNutrientsContinuously();

private:
    PIDManager& pidManager;
    VolumeManager& volumeManager;
    float tempSetpoint;
    float phSetpoint;
    float doSetpoint;
    float nutrientConc;
    float baseConc;
    int duration;
    String experimentName;
    String comment;

    unsigned long startTime;
    unsigned long pauseStartTime;
    unsigned long totalPauseTime;

    void updateVolume();
    void checkCompletion();

    static const int MIN_STIRRING_SPEED = 500;
};

#endif // FERMENTATION_PROGRAM_H