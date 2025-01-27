// PressureSterilizationProgram.h
#ifndef PRESSURE_STERILIZATION_PROGRAM_H
#define PRESSURE_STERILIZATION_PROGRAM_H

#include "ProgramBase.h"
#include "PIDManager.h"
#include "ActuatorController.h"
#include "SensorController.h"
#include "Logger.h"

class PressureSterilizationProgram : public ProgramBase {
public:
    explicit PressureSterilizationProgram(PIDManager& pidManager);
    void start(const String& command) override;
    void update() override;
    void stop() override;
    void pause() override;
    void resume() override;
    bool isRunning() const override { return _isRunning; }
    bool isPaused() const override { return _isPaused; }
    String getName() const override { return "PressureSterilization"; }
    void parseCommand(const String& command) override;
    void getParameters(JsonDocument& doc) const override;

private:
    PIDManager& _pidManager;
};

#endif