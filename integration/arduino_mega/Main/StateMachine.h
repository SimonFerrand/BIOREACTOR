// StateMachine.h
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "ProgramBase.h"
#include "Logger.h"
#include "PIDManager.h"
#include "VolumeManager.h"
#include "SimpleMap.h"
#include "Communication.h"

enum class ProgramState {
    IDLE,
    RUNNING,
    PAUSED,
    COMPLETED,
    STOPPED,
    ERROR
};

class StateMachine {
public:
    StateMachine(PIDManager& pidManager, VolumeManager& volumeManager, Communication& espCommunication);
    void addProgram(const String& name, ProgramBase* program);
    void update();
    void startProgram(const String& programName, const String& command);
    void stopProgram(const String& programName);
    void stopAllPrograms();
    ProgramState getCurrentState() const;
    String getCurrentProgram() const;

private:
    static const int MAX_PROGRAMS = 10;
    SimpleMap<String, ProgramBase*, MAX_PROGRAMS> programs;
    ProgramState currentState;
    ProgramBase* currentProgram;
    PIDManager& pidManager;
    VolumeManager& volumeManager;
    Communication& espCommunication;

    void transitionToState(ProgramState newState);
};

#endif // STATE_MACHINE_H