// StateMachine.cpp
#include "StateMachine.h"


StateMachine::StateMachine(PIDManager& pidManager, MQTTClient& mqttClient)
    : currentState(ProgramState::IDLE)
    , currentProgram(nullptr)
    , pidManager(pidManager)
    , _mqttClient(mqttClient)
{
    _stateMutex = xSemaphoreCreateMutex();
}

void StateMachine::stateMachineTask(void* parameter) {
    StateMachine* machine = static_cast<StateMachine*>(parameter);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (true) {
        machine->update();
        vTaskDelayUntil(&xLastWakeTime, machine->taskFrequency);
    }
}

//begin Task
bool StateMachine::begin() {
    return TaskManager::createTask(
        stateMachineTask,
        "StateMachine",
        STACK_SIZE_MONITOR,
        this,
        TASK_PRIORITY_MEDIUM,
        SENSOR_CORE
    );
}

void StateMachine::addProgram(const String& name, ProgramBase* program) {
    if (program != nullptr) {
        programs.insert(name, program);  
        Logger::log(Logger::LogLevel::INFO, "Program added: " + name);
    }
}

void StateMachine::update() {
    if (xSemaphoreTake(_stateMutex, portMAX_DELAY)) {
        if (currentProgram && currentState == ProgramState::RUNNING) {
            currentProgram->update();
            if (!currentProgram->isRunning()) {
                transitionToState(ProgramState::COMPLETED);
                currentProgram = nullptr;
                Logger::log(Logger::LogLevel::INFO, F("Program completed and cleared"));
            }
        }
        xSemaphoreGive(_stateMutex);
    }
}

/*
void StateMachine::startProgram(const String& programName, const String& command) {
    ProgramBase* program = programs[programName];
    if (program) {
        currentProgram = program;
        currentProgram->start(command);
        transitionToState(ProgramState::RUNNING);
        Logger::log(Logger::LogLevel::INFO, "Started program: " + programName);
        // Envoyer le nouvel état
        String jsonData = DataManager::collectAllData(*this);
        mqttClient.publish(MQTT_TOPIC_SENSORS, 0, true, jsonData.c_str());
    } else {
        Logger::log(Logger::LogLevel::WARNING, "Program not found: " + programName);
    }
}
*/

void StateMachine::startProgram(const String& programName, const String& command) {
    if (xSemaphoreTake(_stateMutex, portMAX_DELAY)) {
        ProgramBase** program = programs.find(programName);
        if (program) {
            currentProgram = *program;
            currentProgram->start(command);
            transitionToState(ProgramState::RUNNING);
            Logger::log(Logger::LogLevel::INFO, "Started program: " + programName);
            // Envoyer le nouvel état
            _mqttClient.publishStateChange(*this);
        } else {
            Logger::log(Logger::LogLevel::WARNING, "Program not found: " + programName);
        }
        xSemaphoreGive(_stateMutex);
    }
}

/*
void StateMachine::startProgram(const String& programName, const String& command) {
    ProgramBase** program = programs.find(programName);
    if (program) {
        //stopProgram();
        currentProgram = *program;
        currentProgram->start(command);
        transitionToState(ProgramState::RUNNING);
        // log and send the data
        //Logger::logProgramEvent(programName, currentProgram);
        //espCommunication.sendProgramEvent(programName, currentProgram);
        Logger::log(Logger::LogLevel::INFO, "Started program: " + programName);
    } else {
        Logger::log(Logger::LogLevel::WARNING, "Program not found: " + programName);
    }
}
*/

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
        Logger::log(Logger::LogLevel::INFO, "Stopped program: " + programName);
        // Envoyer le nouvel état
        _mqttClient.publishStateChange(*this);
    }
}


void StateMachine::stopAllPrograms() {
    if (currentProgram) {
        //Logger::log(Logger::LogLevel::INFO, "Stopping current program: " + currentProgram->getName());
        currentProgram->stop();
        currentProgram = nullptr; 
        transitionToState(ProgramState::STOPPED);
        Logger::log(Logger::LogLevel::INFO, F("All programs stopped"));
        Logger::log(Logger::LogLevel::INFO, "Current state: " + String(static_cast<int>(getCurrentState()))); 
        Logger::log(Logger::LogLevel::INFO, F("Current program: None"));  
        // Envoyer le nouvel état
        _mqttClient.publishStateChange(*this);
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
        Logger::log(Logger::LogLevel::INFO, "State changed to: " + String(static_cast<int>(currentState)));

        // Envoyer TOUS les changement d'états
        //String jsonData = DataManager::collectAllData(*this);
        //mqttClient.publish(MQTT_TOPIC_SENSORS, 0, true, jsonData.c_str());
    }
}

