// CommandHandler.cpp
#include "CommandHandler.h"
#include "StateMachine.h"
#include "MQTTClient.h"
#include "DataManager.h"
#include "Logger.h"

CommandHandler::CommandHandler(StateMachine& stateMachine, MQTTClient& mqttClient)
   : _stateMachine(stateMachine)
   , _mqttClient(mqttClient)
{
}

void CommandHandler::commandTask(void* parameter) {
    CommandHandler* handler = static_cast<CommandHandler*>(parameter);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (true) {
        if (Serial.available() > 0) {
            String command = Serial.readStringUntil('\n');
            command.trim();
            handler->executeCommand(command);
        }
        vTaskDelayUntil(&xLastWakeTime, handler->taskFrequency);
    }
}

bool CommandHandler::begin() {
    return TaskManager::createTask(
        commandTask,
        "CommandHandler",
        STACK_SIZE_MQTT,
        this,
        TASK_PRIORITY_MEDIUM,
        MQTT_CORE
    );
}

void CommandHandler::executeCommand(const String& command) {
   Logger::log(Logger::LogLevel::INFO, "Executing command: " + command);

   if (command == "help") {
       printHelp();
   }
   else if (command.startsWith("cip")) {
       handleCIPCommand(command);
   }
   else if (command == "stop") {
       _stateMachine.stopAllPrograms();
       Logger::log(Logger::LogLevel::INFO, F("Program stopped"));
   }
   else {
       Logger::log(Logger::LogLevel::WARNING, "Unknown command: " + command);
   }
}

void CommandHandler::handleCIPCommand(const String& command) {
   _stateMachine.startProgram("CIP", command);
}

void CommandHandler::printHelp() {
   Serial.println(F("Available commands:"));
   Serial.println(F("cip <temp> <duration> - Start CIP program"));
   Serial.println(F("stop - Stop current program"));
   Serial.println(F("help - Show this help message"));
   Serial.println(F("\nHTTP Endpoints:"));
   Serial.println(F("GET http://<ip>/  - Main dashboard"));
   Serial.println(F("GET http://<ip>/data  - Current sensor data"));
   Serial.println(F("GET http://<ip>/program  - Programs control page")); 
   Serial.println(F("GET http://<ip>/api/data  - Raw JSON data"));
   Serial.println(F("GET http://<ip>/api/status - System status"));
   Serial.println(F("GET http://<ip>/api/system - System info"));
   Serial.println(F("GET http://<ip>/update - OTA firmware update"));
   Serial.println(F("\nProgram Control URLs:")); 
   Serial.println(F("GET http://<ip>/cip?temp=XX&duration=YY - Start CIP program; ex: 'http://192.168.1.45/cip?temp=30&duration=30"));
   Serial.println(F("GET http://<ip>/stop - Stop all programs"));
}

void CommandHandler::handleJsonCommand(const String& jsonCommand) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonCommand);
    
    if (error) {
        Logger::log(Logger::LogLevel::ERROR, F("Invalid JSON command format"));
        return;
    }
    
    String command = doc["command"].as<String>();
    
    if (command == "setParams") {
        Logger::log(Logger::LogLevel::INFO, F("Processing setParams command"));
    } 
    else if (command == "getData") {
        SensorData data = DataManager::collectSensorData();
        _mqttClient.publishSensorData(data);
    }
    else if (command == "cip") {
        JsonObject params = doc["params"].as<JsonObject>();
        String cipCmd = "cip " + String(params["temp"].as<float>()) + " " + 
                       String(params["duration"].as<int>());
        handleCIPCommand(cipCmd);
    }
    else {
        Logger::log(Logger::LogLevel::WARNING, "Unknown command: " + command);
    }
}