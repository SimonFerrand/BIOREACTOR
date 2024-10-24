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
    } else if (command.startsWith("o2 cal ")) {
        handleO2CalibrationCommand(command.substring(7));
    } else if (command == "o2 status") {
        float waterTemp = SensorController::readSensor("waterTempSensor");
        Serial7.println("O2:READ:" + String(waterTemp, 1));
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
    float waterTemp = SensorController::readSensor("waterTempSensor");
    
    if (command == "start") {
        o2CalState = O2CalibrationState::WAITING_ZERO;
        Logger::log(LogLevel::INFO, F("Starting O2 calibration"));
        Logger::log(LogLevel::INFO, F("1. Prepare zero solution (2g Na2S2O3 in 100mL)"));
        Logger::log(LogLevel::INFO, F("   Wait 1 hour and type 'o2 cal zero' when ready"));
    }
    else if (command == "zero" && o2CalState == O2CalibrationState::WAITING_ZERO) {
        Serial7.println("O2:CAL:ZERO:" + String(waterTemp, 1));
        o2CalState = O2CalibrationState::WAITING_SAT_LOW;
        Logger::log(LogLevel::INFO, F("Zero point saved. Temperature: ") + String(waterTemp, 1) + F("°C"));
        Logger::log(LogLevel::INFO, F("2. Prepare room temperature water (20-25°C)"));
        Logger::log(LogLevel::INFO, F("   Aerate for 15min minimum"));
        Logger::log(LogLevel::INFO, F("   Type 'o2 cal low' when ready"));
    }
    else if (command == "low" && o2CalState == O2CalibrationState::WAITING_SAT_LOW) {
        Serial7.println("O2:CAL:SAT_LOW:" + String(waterTemp, 1));
        o2CalState = O2CalibrationState::WAITING_SAT_HIGH;
        Logger::log(LogLevel::INFO, F("Low temperature point saved. Temperature: ") + String(waterTemp, 1) + F("°C"));
        Logger::log(LogLevel::INFO, F("3. Heat water to ~35°C while aerating"));
        Logger::log(LogLevel::INFO, F("   Type 'o2 cal high' when ready"));
    }
    else if (command == "high" && o2CalState == O2CalibrationState::WAITING_SAT_HIGH) {
        Serial7.println("O2:CAL:SAT_HIGH:" + String(waterTemp, 1));
        o2CalState = O2CalibrationState::COMPLETED;
        Logger::log(LogLevel::INFO, F("High temperature point saved. Temperature: ") + String(waterTemp, 1) + F("°C"));
        Logger::log(LogLevel::INFO, F("Calibration completed!"));
    }
    else if (command == "reset") {
        Serial7.println("O2:CAL:RESET");
        o2CalState = O2CalibrationState::NONE;
        Logger::log(LogLevel::INFO, F("Calibration reset"));
    }
    else if (command == "status") {
        Serial7.println("O2:CAL:STATUS");
    }
    else {
        Logger::log(LogLevel::WARNING, F("Invalid O2 calibration command"));
        Logger::log(LogLevel::INFO, F("Available commands:"));
        Logger::log(LogLevel::INFO, F("  o2 cal start - Start calibration"));
        Logger::log(LogLevel::INFO, F("  o2 cal zero  - Save zero point"));
        Logger::log(LogLevel::INFO, F("  o2 cal low   - Save low temp saturation"));
        Logger::log(LogLevel::INFO, F("  o2 cal high  - Save high temp saturation"));
        Logger::log(LogLevel::INFO, F("  o2 cal reset - Reset calibration"));
        Logger::log(LogLevel::INFO, F("  o2 cal status - Show calibration status"));
    }
}

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
    Serial.println("    fillPump <speed_0_100%> <duration_seconds>");
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
    Serial.println(F("O2 Calibration commands:"));
    Serial.println(F("  o2 cal start - Start calibration procedure"));
    Serial.println(F("  o2 cal zero  - Calibrate zero point"));
    Serial.println(F("  o2 cal low   - Calibrate low temp saturation"));
    Serial.println(F("  o2 cal high  - Calibrate high temp saturation"));
    Serial.println(F("  o2 cal reset - Reset calibration"));
    Serial.println(F("  o2 status    - Show calibration status"));
    Serial.println(F("-----------------------------------------------------------------------------------------------------------------------"));
}


