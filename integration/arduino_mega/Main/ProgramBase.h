// ProgramBase.h
#ifndef PROGRAM_BASE_H
#define PROGRAM_BASE_H

#include <Arduino.h>

class ProgramBase {
public:
    // Start the program with the given command
    virtual void start(const String& command) = 0;

    // Update the program state (called in the main loop)
    virtual void update() = 0;

    // Stop the program
    virtual void stop() = 0;

    // Pause the program execution
    virtual void pause() = 0;

    // Resume the program execution
    virtual void resume() = 0;

    // Check if the program is currently running
    virtual bool isRunning() const = 0;

    // Check if the program is currently paused
    virtual bool isPaused() const = 0;

    // Get the name of the programme
    virtual String getName() const = 0;

    virtual void parseCommand(const String& command) = 0;

    // Virtual destructor
    virtual ~ProgramBase() {}
    

protected:
    bool _isRunning = false;
    bool _isPaused = false;
};

#endif // PROGRAM_BASE_H