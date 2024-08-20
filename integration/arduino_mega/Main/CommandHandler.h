#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include "StateMachine.h"
#include "SafetySystem.h"
#include "VolumeManager.h"
#include "Logger.h"
#include "PIDManager.h"

class CommandHandler {
public:
    CommandHandler(StateMachine& stateMachine, SafetySystem& safetySystem, 
                   VolumeManager& volumeManager, Logger& logger,
                   PIDManager& pidManager);

    void executeCommand(const String& command);
    void printHelp();
    static float getPumpMaxFlowRate(const String& actuatorName);

private:
    StateMachine& stateMachine;
    SafetySystem& safetySystem;
    VolumeManager& volumeManager;
    Logger& logger;
    PIDManager& pidManager;

    void handleAdjustVolume(const String& command);

    void handleSetCommand(const String& command);
    
    void handlePHCalibrationCommand(const String& command);

    void handleVolumeInfoCommand();
};

#endif // COMMAND_HANDLER_H