// StateMachine.cpp
#include "StateMachine.h"

StateMachine::StateMachine(Logger& logger, PIDManager& pidManager, VolumeManager& volumeManager)
    : currentState(ProgramState::IDLE),
      currentProgram(nullptr),
      logger(logger),
      pidManager(pidManager),
      volumeManager(volumeManager)
{
}

void StateMachine::addProgram(const String& name, ProgramBase* program) {
    if (programs.getSize() < MAX_PROGRAMS) {
        programs.insert(name, program);
    } else {
        logger.log(LogLevel::ERROR, "Maximum number of programs reached");
    }
}

void StateMachine::update() {
    if (currentProgram && currentState == ProgramState::RUNNING) {
        currentProgram->update();
        if (!currentProgram->isRunning()) {
            transitionToState(ProgramState::COMPLETED);
            currentProgram = nullptr; 
            Logger::log(LogLevel::INFO, "Program completed and cleared");
        }
    }
}

void StateMachine::startProgram(const String& programName, const String& command) {
    ProgramBase** program = programs.find(programName);
    if (program) {
        //stopProgram();
        currentProgram = *program;
        currentProgram->start(command);
        transitionToState(ProgramState::RUNNING);
        logger.log(LogLevel::INFO, "Started program: " + programName);
    } else {
        logger.log(LogLevel::WARNING, "Program not found: " + programName);
    }
}

// This method stops the currently running program if it matches the given program name
void StateMachine::stopProgram(const String& programName) {   //TEST
  // Check if there is a current program running and if its name matches the given program name
    if (currentProgram && currentProgram->getName() == programName) {
      // Stop the current program by calling its stop method
        currentProgram->stop();

        currentProgram = nullptr;
        // Transition the state machine to the STOPPED state
        transitionToState(ProgramState::STOPPED);
        // Log the action of stopping the program
        Logger::log(LogLevel::INFO, "Stopped program: " + programName);
    }
}


void StateMachine::stopAllPrograms() {
    if (currentProgram) {
        Logger::log(LogLevel::INFO, "Stopping current program: " + currentProgram->getName());
        currentProgram->stop();
        currentProgram = nullptr; 
        transitionToState(ProgramState::STOPPED);
        Logger::log(LogLevel::INFO, "All programs stopped");
        //Logger::log(LogLevel::INFO, "Current state: " + programStateToString(getCurrentState()));
        Logger::log(LogLevel::INFO, "Current state: " + String(static_cast<int>(getCurrentState())));
        Logger::log(LogLevel::INFO, "Current program: None");  // Modifiez cette ligne
    }
}

ProgramState StateMachine::getCurrentState() const {
    return currentState;
}

String StateMachine::getCurrentProgram() const {
    return currentProgram ? currentProgram->getName() : "None";
}

void StateMachine::transitionToState(ProgramState newState) {
    if (newState != currentState) {
        currentState = newState;
        Logger::log(LogLevel::INFO, "State changed to: " + String(static_cast<int>(currentState)));
    }
}

