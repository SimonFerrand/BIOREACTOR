#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

// Forward declarations
class StateMachine;
class MQTTClient;

class CommandHandler {
public:
    CommandHandler(StateMachine& stateMachine, MQTTClient& mqttClient);
    void executeCommand(const String& command);
    void handleJsonCommand(const String& jsonCommand);
    void handleCIPCommand(const String& command);
    bool begin();
    
private:
    StateMachine& _stateMachine;
    MQTTClient& _mqttClient;
    void printHelp();

    // Task
    TaskHandle_t taskHandle;
    static void commandTask(void* parameter);
    const TickType_t taskFrequency = pdMS_TO_TICKS(TASK_INTERVAL_COMMAND);
};

#endif