// CommandHandler.cpp
#include "CommandHandler.h"

#include "CommandHandler.h"

CommandHandler::CommandHandler(StateMachine& stateMachine, SafetySystem& safetySystem, 
                               VolumeManager& volumeManager, Logger& logger,
                               PIDManager& pidManager)
    : stateMachine(stateMachine), safetySystem(safetySystem), 
      volumeManager(volumeManager), logger(logger),
      pidManager(pidManager) {}

void CommandHandler::executeCommand(const String& command) {
    logger.log(LogLevel::INFO, "Executing command: " + command);

    if (command == "help") {
        printHelp();
    } else if (command.startsWith("test") || command == "tests") {
        stateMachine.startProgram("Tests", command);
    } else if (command.startsWith("drain")) {
        stateMachine.startProgram("Drain", command);
    } else if (command.startsWith("mix")) {
        stateMachine.startProgram("Mix", command);
    } else if (command.startsWith("fermentation")) {
        stateMachine.startProgram("Fermentation", command);
    } else if (command == "stop") {
        stateMachine.stopAllPrograms();
        //stateMachine.stopProgram();
    } else if (command.startsWith("adjust_volume")) {
        handleAdjustVolume(command);
    } else if (command.startsWith("set_") || command.startsWith("alarms ") || command.startsWith("warnings ")) {
        handleSetCommand(command);
    } else {
        logger.log(LogLevel::WARNING, "Unknown command: " + command);
    }
}

void CommandHandler::handleAdjustVolume(const String& command) {
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);
    if (firstSpace != -1 && secondSpace != -1) {
        String source = command.substring(firstSpace + 1, secondSpace);
        float amount = command.substring(secondSpace + 1).toFloat();
        volumeManager.manuallyAdjustVolume(amount, source);
        logger.log(LogLevel::INFO, "Manual volume adjustment: " + source + " " + String(amount));
    } else {
        logger.log(LogLevel::WARNING, "Invalid adjust_volume command format");
    }
}

void CommandHandler::handleSetCommand(const String& command) {
    if (command.startsWith("alarms ") || command.startsWith("warnings ")) {
        safetySystem.parseCommand(command);
    } else if (command.startsWith("set_check_interval")) {
        int interval = command.substring(19).toInt();
        safetySystem.setCheckInterval(interval * 1000); // Convert to milliseconds
        logger.log(LogLevel::INFO, "Safety check interval set to " + String(interval) + " seconds");
    } else if (command.startsWith("set_initial_volume")) {
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex != -1) {
            float initialVolume = command.substring(spaceIndex + 1).toFloat();
            volumeManager.setInitialVolume(initialVolume);
            logger.log(LogLevel::INFO, "Initial culture volume set to " + String(initialVolume) + " L");
        } else {
            logger.log(LogLevel::WARNING, "Invalid set_initial_volume command. Usage: set_initial_volume <volume_in_liters>");
        }
    } else {
        logger.log(LogLevel::WARNING, "Unknown set command: " + command);
    }
}



void CommandHandler::printHelp() {
    Serial.println();
    Serial.println("------------------------------------------------- Available commands: -------------------------------------------------");
    Serial.println("help - Display this help message");
    Serial.println("test sensors - Start continuous sensor data reading");
    Serial.println("test <actuator> <value> <duration> - Test a specific actuator");
    Serial.println("  Available actuators:");
    Serial.print("    basePump <flow_rate_0_");
    Serial.print(ActuatorController::getPumpMaxFlowRate("basePump"), 1);  // 1 decimal place
    Serial.println("_ml_per_min> <duration_seconds>");
    Serial.print("    nutrientPump <flow_rate_0_");
    Serial.print(ActuatorController::getPumpMaxFlowRate("nutrientPump"), 1);  // 1 decimal place
    Serial.println("_ml_per_min> <duration_seconds>");
    Serial.println("    airPump <speed_0_100%> <duration_seconds>");
    Serial.println("    drainPump <speed_0_100%> <duration_seconds>");
    Serial.print("    stirringMotor <speed_");
    Serial.print(ActuatorController::getStirringMotorMinRPM());
    Serial.print("_");
    Serial.print(ActuatorController::getStirringMotorMaxRPM());
    Serial.println("> <duration_seconds>");
    Serial.println("    heatingPlate <power_0_100%> <duration_seconds>");
    Serial.println("    ledGrowLight <intensity_0_100%> <duration_seconds>");
    Serial.println("tests - Run all predefined tests");
    Serial.println("drain <rate> <duration> - Start draining");
    Serial.println("stop - Stop all actuators and PIDs");
    Serial.println("mix <speed> - Start mixing");
    Serial.println("fermentation <temp> <ph> <do> <nutrient_conc> <base_conc> <duration> <experiment_name> <comment> - Start fermentation");
    Serial.println("test pid <type> <setpoint> - Start PID control (type: temp, ph, or do)");
    Serial.println("adjust_volume <source> <amount> - Manually adjust volume");
    Serial.println("alarm false - Disable safety alarms");
    Serial.println("alarm true - Enable safety alarms");
    Serial.println("warning false - Disable safety warnings");
    Serial.println("warning true - Enable safety warnings");
    Serial.println("set_check_interval <seconds> - Set safety check interval");
    Serial.println("set_initial_volume <volume> - Set the initial culture volume (in liters)");
    Serial.println("-----------------------------------------------------------------------------------------------------------------------");
}

