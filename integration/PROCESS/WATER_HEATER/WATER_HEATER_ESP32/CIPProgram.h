// CIPProgram.h
#ifndef CIP_PROGRAM_H  
#define CIP_PROGRAM_H

#include "ProgramBase.h"
#include "PIDManager.h"
#include "Logger.h"

class CIPProgram : public ProgramBase {
public:
   explicit CIPProgram(PIDManager& pidManager);
   void start(const String& command) override;
   void update() override;
   void stop() override;
   void pause() override;
   void resume() override;
   bool isRunning() const override { return _isRunning; }
   bool isPaused() const override { return _isPaused; }
   String getName() const override { return "CIP"; }
   void parseCommand(const String& command) override;
   void getParameters(JsonDocument& doc) const override;

private:
   PIDManager& _pidManager;
   float targetTemp;
   unsigned long duration;
   unsigned long startTime;
};

#endif