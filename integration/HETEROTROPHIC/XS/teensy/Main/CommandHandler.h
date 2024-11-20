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
                   VolumeManager& volumeManager, PIDManager& pidManager);
                   

    void executeCommand(const String& command);
    void printHelp();
    static float getPumpMaxFlowRate(const String& actuatorName);
    

private:
    StateMachine& stateMachine;
    SafetySystem& safetySystem;
    VolumeManager& volumeManager;
    PIDManager& pidManager;

    // O2 calibration states
    enum class O2CalibrationState {
        NONE,
        WAITING_ZERO,
        WAITING_SAT_LOW,
        WAITING_SAT_HIGH,
        COMPLETED
    };
    O2CalibrationState o2CalState = O2CalibrationState::NONE;

    void handleAdjustVolume(const String& command);

    void handleSetCommand(const String& command);
    
    void handlePHCalibrationCommand(const String& command);

    void handleVolumeInfoCommand();

    void handleO2CalibrationCommand(const String& command);

    String sendCommandAndWaitResponse(const String& cmd) {
        Serial7.println(cmd);
        unsigned long startTime = millis();
        while (!Serial7.available()) {
            if (millis() - startTime > 5000) {
                Logger::log(LogLevel::ERROR, "Timeout waiting for Arduino response");
                return "ERROR:TIMEOUT";
            }
        }
        return Serial7.readStringUntil('\n');
    }


};

#endif // COMMAND_HANDLER_H