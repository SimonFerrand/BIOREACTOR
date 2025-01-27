// StateMachine.h
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "ProgramBase.h"
#include "Logger.h"
#include "PIDManager.h"
#include <map>
#include "TaskManager.h"
#include "config.h"
#include "SimpleMap.h"
#include "DataManager.h"
#include "MQTTClient.h"
//#include "VolumeManager.h"
//#include "Communication.h"

enum class ProgramState {
    IDLE,
    RUNNING,
    PAUSED,
    COMPLETED,
    STOPPED,
    ERROR
};

class WebAPIHandler;
class MQTTClient;

class StateMachine {
public:
    StateMachine(PIDManager& pidManager, MQTTClient& mqttClient);
                 //,VolumeManager& volumeManager, 
                //Communication& espCommunication
                
    void update();
    void startProgram(const String& programName, const String& command);
    void stopProgram(const String& programName);
    void stopAllPrograms();
    String getCurrentProgram() const;
    ProgramState getCurrentState() const;
    void addProgram(const String& name, ProgramBase* program);
    bool begin();

private:
    ProgramState currentState;
    ProgramBase* currentProgram;
    PIDManager& pidManager;
    MQTTClient& _mqttClient;
    //VolumeManager& volumeManager;
    //Communication& espCommunication;
    SemaphoreHandle_t _stateMutex;
    static const int MAX_PROGRAMS = 5;
    SimpleMap<String, ProgramBase*, MAX_PROGRAMS> programs;


    void transitionToState(ProgramState newState);

    //Task
    TaskHandle_t taskHandle;
    static void stateMachineTask(void* parameter);
    const TickType_t taskFrequency = pdMS_TO_TICKS(TASK_INTERVAL_STATEMACHINE);
};

#endif // STATE_MACHINE_H