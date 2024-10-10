// CommandHandler.cpp
#include "CommandHandler.h"

CommandHandler::CommandHandler(StateMachine& stateMachine, SafetySystem& safetySystem, 
                               VolumeManager& volumeManager, PIDManager& pidManager)
                              
    : stateMachine(stateMachine), safetySystem(safetySystem), 
      volumeManager(volumeManager), pidManager(pidManager) {}
      

void CommandHandler::executeCommand(const String& command) {
    Logger::log(LogLevel::INFO, "Executing command: " + command);

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
    } else if (command.startsWith("ph ")) {
        handlePHCalibrationCommand(command);
    } else if (command == "volume info") {
        handleVolumeInfoCommand();
    } else if (command.startsWith("o2 calibrate")) {
        handleO2CalibrationCommand(command);
    } else {
        Logger::log(LogLevel::WARNING, "Unknown command: " + command);
    }
}

void CommandHandler::handleAdjustVolume(const String& command) {
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);
    if (firstSpace != -1 && secondSpace != -1) {
        String source = command.substring(firstSpace + 1, secondSpace);
        float amount = command.substring(secondSpace + 1).toFloat();
        volumeManager.manuallyAdjustVolume(amount, source);
        Logger::log(LogLevel::INFO, "Manual volume adjustment: " + source + " " + String(amount));
    } else {
        Logger::log(LogLevel::WARNING, F("Invalid adjust_volume command format"));
    }
}

void CommandHandler::handleSetCommand(const String& command) {
    if (command.startsWith("alarms ") || command.startsWith("warnings ")) {
        safetySystem.parseCommand(command);
    } else if (command.startsWith("set_check_interval")) {
        int interval = command.substring(19).toInt();
        safetySystem.setCheckInterval(interval * 1000); // Convert to milliseconds
        Logger::log(LogLevel::INFO, "Safety check interval set to " + String(interval) + " seconds");
    } else if (command.startsWith("set_initial_volume")) {
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex != -1) {
            float initialVolume = command.substring(spaceIndex + 1).toFloat();
            volumeManager.setInitialVolume(initialVolume);
            Logger::log(LogLevel::INFO, "Initial culture volume set to " + String(initialVolume) + " L");
        } else {
            Logger::log(LogLevel::WARNING, F("Invalid set_initial_volume command. Usage: set_initial_volume <volume_in_liters>"));
        }
    } else {
        Logger::log(LogLevel::WARNING, "Unknown set command: " + command);
    }
}

void CommandHandler::handlePHCalibrationCommand(const String& command) {
    PHSensor* phSensor = (PHSensor*)SensorController::findSensorByName("phSensor");
    if (phSensor) {
        if (command == "ph ENTERPH") {
            phSensor->enterCalibration();
        } else if (command == "ph CALPH") {
            phSensor->calibrate();
        } else if (command == "ph EXITPH") {
            phSensor->exitCalibration();
        } else {
            Logger::log(LogLevel::WARNING, "Invalid pH calibration command: " + command);
        }
    } else {
        Logger::log(LogLevel::WARNING, "pH sensor not found");
    }
}

void CommandHandler::handleVolumeInfoCommand() {
    String volumeInfo = volumeManager.getVolumeInfo();
    Logger::log(LogLevel::INFO, volumeInfo);
}

void CommandHandler::handleO2CalibrationCommand(const String& command) {
    OxygenSensor* o2Sensor = (OxygenSensor*)SensorController::findSensorByName("oxygenSensor");
    if (o2Sensor) {
        if (command.startsWith("o2 calibrate start")) {
            int points = command.substring(19).toInt();
            o2Sensor->startCalibration(points);
        } else if (command == "o2 calibrate save") {
            o2Sensor->saveCalibrationPoint();
        } else if (command == "o2 calibrate finish") {
            o2Sensor->finishCalibration();
        } else if (command == "o2 calibrate status") {
            Logger::log(LogLevel::INFO, o2Sensor->getCalibrationStatus());
        } else if (command == "o2 calibrate reset") {
            o2Sensor->resetCalibration();
        } else {
            Logger::log(LogLevel::WARNING, String(F("Invalid O2 calibration command: ")) + command);
        }
    } else {
        Logger::log(LogLevel::WARNING, F("Oxygen sensor not found"));
    }
}

  /*
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
    Serial.println("    samplePump <speed_0_100%> <duration_seconds>");
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
    Serial.println("alarm false - Disable safety alarms");
    Serial.println("alarm true - Enable safety alarms");
    Serial.println("warning false - Disable safety warnings");
    Serial.println("warning true - Enable safety warnings");
    Serial.println("set_check_interval <seconds> - Set safety check interval");
    Serial.println("adjust_volume <source> <amount> - Manually adjust volume (source: NaOH, Nutrient, Microalgae, Removed; amount in liter");
    Serial.println("set_initial_volume <volume> - Set the initial culture volume (in liters)");
    Serial.println("ph ENTERPH - Enter pH calibration mode");
    Serial.println("ph CALPH - Calibrate with buffer solution");
    Serial.println("ph EXITPH - Save and exit pH calibration mode");
    Serial.println("-----------------------------------------------------------------------------------------------------------------------");
}
*/

void CommandHandler::printHelp() {
    Serial.println();
    Serial.println(F("------------------------------------------------- Available commands: -------------------------------------------------"));
    Serial.println(F("help - Display this help message"));
    Serial.println(F("test sensors - Start continuous sensor data reading"));
    Serial.println(F("test <actuator> <value> <duration> - Test a specific actuator"));
    Serial.println(F("  Available actuators:"));
    Serial.print(F("    basePump <flow_rate_0_"));
    Serial.print(ActuatorController::getPumpMaxFlowRate("basePump"), 1);  // 1 decimal place
    Serial.println(F("_ml_per_min> <duration_seconds>"));
    Serial.print(F("    nutrientPump <flow_rate_0_"));
    Serial.print(ActuatorController::getPumpMaxFlowRate("nutrientPump"), 1);  // 1 decimal place
    Serial.println(F("_ml_per_min> <duration_seconds>"));
    Serial.println(F("    airPump <speed_0_100%> <duration_seconds>"));
    Serial.println(F("    drainPump <speed_0_100%> <duration_seconds>"));
    Serial.println(F("    samplePump <speed_0_100%> <duration_seconds>"));
    Serial.print(F("    stirringMotor <speed_"));
    Serial.print(ActuatorController::getStirringMotorMinRPM());
    Serial.print(F("_"));
    Serial.print(ActuatorController::getStirringMotorMaxRPM());
    Serial.println(F("> <duration_seconds>"));
    Serial.println(F("    heatingPlate <power_0_100%> <duration_seconds>"));
    Serial.println(F("    ledGrowLight <intensity_0_100%> <duration_seconds>"));
    Serial.println(F("tests - Run all predefined tests"));
    Serial.println(F("drain <rate> <duration> - Start draining"));
    Serial.println(F("stop - Stop all actuators and PIDs"));
    Serial.println(F("mix <speed> - Start mixing"));
    Serial.println(F("fermentation <temp> <ph> <do> <nutrient_conc> <base_conc> <duration> <experiment_name> <comment> - Start fermentation"));
    Serial.println(F("test pid <type> <setpoint> - Start PID control (type: temp, ph, or do)"));
    Serial.println(F("alarm false - Disable safety alarms"));
    Serial.println(F("alarm true - Enable safety alarms"));
    Serial.println(F("warning false - Disable safety warnings"));
    Serial.println(F("warning true - Enable safety warnings"));
    Serial.println(F("set_check_interval <seconds> - Set safety check interval"));
    Serial.println(F("adjust_volume <source> <amount> - Manually adjust volume (source: NaOH, Nutrient, Microalgae, Removed; amount in liter)"));
    Serial.println(F("set_initial_volume <volume> - Set the initial culture volume (in liters)"));
    Serial.println(F("ph ENTERPH - Enter pH calibration mode : put the probe into the 4.0 or 7.0 standard buffer solution" ));
    Serial.println(F("ph CALPH - Calibrate with buffer solution : standard buffer solution will be detected automatically "));
    Serial.println(F("ph EXITPH - Save and exit pH calibration mode"));
    Serial.println(F("volume info - Get all volume informations"));
    Serial.println(F("set_pid_enabled - set pid enabled during Fermentation program (true, false "));
    Serial.println(F("o2 calibrate start <points> - Start O2 calibration with specified number of points (1-3)"));
    Serial.println(F("o2 calibrate save - Save current O2 calibration point"));
    Serial.println(F("o2 calibrate finish - Finish and save O2 calibration"));
    Serial.println(F("o2 calibrate status - Display current O2 calibration status"));
    Serial.println(F("o2 calibrate reset - Reset O2 calibration data"));
    Serial.println(F("-----------------------------------------------------------------------------------------------------------------------"));
}


