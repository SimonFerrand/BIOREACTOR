// DrainProgram.h
#ifndef DRAIN_PROGRAM_H
#define DRAIN_PROGRAM_H

#include "ProgramBase.h"
#include "ActuatorController.h"
#include "Logger.h"

class DrainProgram : public ProgramBase {
public:
    DrainProgram();
    void start(const String& command) override;
    void update() override;
    void pause() override;
    void resume() override;
    void stop() override;
    bool isRunning() const override { return _isRunning; }
    bool isPaused() const override { return _isPaused; }
    String getName() const { return "Drain"; }
    void parseCommand(const String& command) override;

private:
    int rate;
    int duration;
    unsigned long startTime;
};

#endif // DRAIN_PROGRAM_H