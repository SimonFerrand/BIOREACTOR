// MixProgram.h
#ifndef MIX_PROGRAM_H
#define MIX_PROGRAM_H

#include "ProgramBase.h"
#include "ActuatorController.h"
#include "Logger.h"

class MixProgram : public ProgramBase {
public:
    MixProgram();
    void start(const String& command) override;
    void update() override;
    void pause() override;
    void resume() override;
    void stop() override;
    bool isRunning() const override { return _isRunning; }
    bool isPaused() const override { return _isPaused; }
    String getName() const { return "Mix"; }
    void parseCommand(const String& command) override;

private:
    int speed;
};

#endif // MIX_PROGRAM_H