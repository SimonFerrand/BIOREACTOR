// PIDManager.cpp
#include "PIDManager.h"
#include "Logger.h"
#include <Arduino.h>
#include "ActuatorController.h"

PIDManager::PIDManager()
    : tempPID(&tempInput, &tempOutput, &tempSetpoint, 0, 0, 0, DIRECT),
      phPID(&phInput, &phOutput, &phSetpoint, 0, 0, 0, DIRECT),
      doPID(&doInput, &doOutput, &doSetpoint, 0, 0, 0, DIRECT),
      tempPIDRunning(false), phPIDRunning(false), doPIDRunning(false),
      lastTempUpdateTime(0), lastPHUpdateTime(0), lastDOUpdateTime(0),
      tempHysteresis(0.5), phHysteresis(0.05), doHysteresis(1.0),
      minStirringSpeed(0),
      isStartupPhase(true)
{
    tempPID.SetOutputLimits(0, 100);
    phPID.SetOutputLimits(0, 100);
    doPID.SetOutputLimits(20, 100); // minimum 20% because AirPump does not work below this level
    tempPID.SetMode(AUTOMATIC);
    phPID.SetMode(AUTOMATIC);
    doPID.SetMode(AUTOMATIC);
}

void PIDManager::initialize(double tempKp, double tempKi, double tempKd,
                            double phKp, double phKi, double phKd,
                            double doKp, double doKi, double doKd) {
    tempPID.SetTunings(tempKp * 1.5, tempKi * 0.5, tempKd * 2);  // Start-up parameters for temperature
    phPID.SetTunings(phKp * 1.5, phKi * 0.5, phKd * 2);  // Start-up parameters for pH
    doPID.SetTunings(doKp * 1.5, doKi * 0.5, doKd * 2);  // Start-up parameters for  DO
}

void PIDManager::setHysteresis(double tempHyst, double phHyst, double doHyst) {
    tempHysteresis = tempHyst;
    phHysteresis = phHyst;
    doHysteresis = doHyst;
}

void PIDManager::updateAllPIDControllers() {
    unsigned long currentTime = millis();
    bool anyPIDUpdated  = false;
    if (tempPIDRunning && currentTime - lastTempUpdateTime >= UPDATE_INTERVAL_TEMP) {
        updateTemperaturePID();
        lastTempUpdateTime = currentTime;
        anyPIDUpdated  = true;
    }
    if (phPIDRunning && currentTime - lastPHUpdateTime >= UPDATE_INTERVAL_PH) {
        updatePHPID();
        lastPHUpdateTime = currentTime;
        anyPIDUpdated  = true;
    }
    if (doPIDRunning && currentTime - lastDOUpdateTime >= UPDATE_INTERVAL_DO) {
        updateDOPID();
        lastDOUpdateTime = currentTime;
        anyPIDUpdated  = true;
    }
    if (anyPIDUpdated ) {
        adjustPIDStirringSpeed();
    }
}

void PIDManager::setTemperatureSetpoint(double setpoint) { tempSetpoint = setpoint; }
void PIDManager::setPHSetpoint(double setpoint) { phSetpoint = setpoint; }
void PIDManager::setDOSetpoint(double setpoint) { doSetpoint = setpoint; }

double PIDManager::getTemperatureOutput() const { return tempOutput; } 
double PIDManager::getPHOutput() const { return phOutput; }             
double PIDManager::getDOOutput() const { return doOutput; }

void PIDManager::startTemperaturePID(double setpoint) {
    tempSetpoint = setpoint;
    tempPIDRunning = true;
    isStartupPhase = true;
    Logger::log(LogLevel::INFO, "Temperature PID started with setpoint: " + String(setpoint));
}

void PIDManager::startPHPID(double setpoint) {
    phSetpoint = setpoint;
    phPIDRunning = true;
    isStartupPhase = true;
    Logger::log(LogLevel::INFO, "pH PID started with setpoint: " + String(setpoint));
}

void PIDManager::startDOPID(double setpoint) {
    doSetpoint = setpoint;
    doPIDRunning = true;
    isStartupPhase = true;
    Logger::log(LogLevel::INFO, "DO PID started with setpoint: " + String(setpoint));
}


void PIDManager::switchToMaintainMode() {
    isStartupPhase = false;
    double tempKp = tempPID.GetKp();
    double tempKi = tempPID.GetKi();
    double tempKd = tempPID.GetKd();
    tempPID.SetTunings(tempKp * 0.5, tempKi * 0.1, tempKd * 2);
    Logger::log(LogLevel::INFO, "Switched to maintain mode for temperature control");
}

/*
void PIDManager::switchToMaintainMode() {
    // Temperature PID
    double tempKp = tempPID.GetKp();
    double tempKi = tempPID.GetKi();
    double tempKd = tempPID.GetKd();

    if (ActuatorController::getHeatingControlMode() == HeatingControlMode::PWM) {
        tempPID.SetTunings(tempKp * 0.5, tempKi * 0.5, tempKd * 0.5);
        tempPID.SetOutputLimits(0, 255);
    } else {
        tempPID.SetTunings(tempKp * 2, tempKi * 0.1, tempKd * 5);
        tempPID.SetOutputLimits(0, 255);  // Keep 0-255 even for Relay mode
    }

    // pH PID
    double phKp = phPID.GetKp();
    double phKi = phPID.GetKi();
    double phKd = phPID.GetKd();
    phPID.SetTunings(phKp * 0.7, phKi * 0.3, phKd * 1.5);

    // DO PID
    double doKp = doPID.GetKp();
    double doKi = doPID.GetKi();
    double doKd = doPID.GetKd();
    doPID.SetTunings(doKp * 0.7, doKi * 0.3, doKd * 1.5);

    // Reset PID modes
    tempPID.SetMode(MANUAL);
    phPID.SetMode(MANUAL);
    doPID.SetMode(MANUAL);
    tempPID.SetMode(AUTOMATIC);
    phPID.SetMode(AUTOMATIC);
    doPID.SetMode(AUTOMATIC);

    isStartupPhase = false;

    // Ajuster les setpoints en fonction de l'hystérésis
    tempSetpoint = tempInput > tempSetpoint ? tempSetpoint + tempHysteresis / 2 : tempSetpoint - tempHysteresis / 2;
    phSetpoint = phInput > phSetpoint ? phSetpoint + phHysteresis / 2 : phSetpoint - phHysteresis / 2;
    doSetpoint = doInput > doSetpoint ? doSetpoint + doHysteresis / 2 : doSetpoint - doHysteresis / 2;

    Logger::log(LogLevel::INFO, "Switched to maintain mode");
    Logger::log(LogLevel::INFO, "New Temperature PID parameters - Kp: " + String(tempKp) + ", Ki: " + String(tempKi) + ", Kd: " + String(tempKd));
    Logger::log(LogLevel::INFO, "New pH PID parameters - Kp: " + String(phKp) + ", Ki: " + String(phKi) + ", Kd: " + String(phKd));
    Logger::log(LogLevel::INFO, "New DO PID parameters - Kp: " + String(doKp) + ", Ki: " + String(doKi) + ", Kd: " + String(doKd));
    Logger::log(LogLevel::INFO, "Adjusted Temperature setpoint: " + String(tempSetpoint));
    Logger::log(LogLevel::INFO, "Adjusted pH setpoint: " + String(phSetpoint));
    Logger::log(LogLevel::INFO, "Adjusted DO setpoint: " + String(doSetpoint));
}
*/

void PIDManager::adjustPIDStirringSpeed() {
    if (!tempPIDRunning && !phPIDRunning && !doPIDRunning) {
        // If no PID is active, use minimum speed
        int minSpeed = getMinStirringSpeed();
        ActuatorController::runActuator("stirringMotor", minSpeed, 0);
        return;
    }

    double maxOutput = max(max(abs(tempOutput), abs(phOutput)), abs(doOutput));
    int pidSpeed = map(maxOutput, 0, 100, ActuatorController::getStirringMotorMinRPM(), ActuatorController::getStirringMotorMaxRPM());
    int finalSpeed = max(pidSpeed, getMinStirringSpeed());
    finalSpeed = constrain(finalSpeed, ActuatorController::getStirringMotorMinRPM(), ActuatorController::getStirringMotorMaxRPM());
    ActuatorController::runActuator("stirringMotor", finalSpeed, 0);
    
    //Logger::log(LogLevel::INFO, "Adjusted stirring motor speed: " + String(finalSpeed));
}

/*
void PIDManager::updateTemperaturePID() {
    if (!tempPIDRunning) return;
    
    tempInput = SensorController::readSensor("waterTempSensor");
    
    if (abs(tempInput - tempSetpoint) > tempHysteresis) {
        tempPID.Compute();
        if (isStartupPhase && abs(tempInput - tempSetpoint) < 2.0) {
            switchToMaintainMode();
        }
           
        ActuatorController::runActuator("heatingPlate", tempOutput, 0);  
        Logger::log(LogLevel::INFO, "Temperature PID update - Setpoint: " + String(tempSetpoint) + ", Input: " + String(tempInput) + ", Output: " + String(tempOutput) + "%");
    } else {
        stopTemperaturePID();
        //Logger::log(LogLevel::INFO, "Temperature within hysteresis range. Stopping temperature control.");
        Logger::log(LogLevel::INFO, F("Temperature within hysteresis range. Stopping temperature control."));
    }
}
*/

void PIDManager::updateTemperaturePID() {
    if (!tempPIDRunning) return;
    
    tempInput = SensorController::readSensor("waterTempSensor");
    
    if (abs(tempInput - tempSetpoint) > tempHysteresis) {
        tempPID.Compute();  // Calculez toujours la sortie PID

        if (isStartupPhase) {
            if (tempInput < tempSetpoint - 2.0) {
                // Si la température est significativement inférieure à la cible, chauffez à pleine puissance
                ActuatorController::runActuator("heatingPlate", 100, 0);
                Logger::log(LogLevel::INFO, "Aggressive heating: 100%");
            } else {
                ActuatorController::runActuator("heatingPlate", tempOutput, 0);
            }
            
            if (abs(tempInput - tempSetpoint) < 1) {
                switchToMaintainMode();
            }
        } else {
            ActuatorController::runActuator("heatingPlate", tempOutput, 0);
        }
        
        Logger::log(LogLevel::INFO, "Temperature PID update - Setpoint: " + String(tempSetpoint) + 
                    ", Input: " + String(tempInput) + ", Output: " + String(tempOutput) + "%, Startup: " + 
                    String(isStartupPhase ? "Yes" : "No"));
    } else {
        ActuatorController::stopActuator("heatingPlate");  // Just stop heating but keep PID active
        Logger::log(LogLevel::INFO, "Temperature within hysteresis range (" + 
                   String(tempHysteresis) + "°C). Heating paused.");
    }
}

void PIDManager::updatePHPID() {
    if (!phPIDRunning) return;
    
    static unsigned long lastAdjustmentTime = 0;
    const unsigned long adjustmentDelay = 60000; // 1 minute delay between adjustments
    
    phInput = SensorController::readSensor("phSensor");
    
    if (abs(phInput - phSetpoint) > phHysteresis && millis() - lastAdjustmentTime > adjustmentDelay) {
        phPID.Compute();
        double flowRate = convertPIDOutputToFlowRate(phOutput);
        
        // Limit the flow rate
        const double maxAllowedFlowRate = 10.0; // ml/min, adjust as needed
        flowRate = min(flowRate, maxAllowedFlowRate);
        
        // Activate pump for a short duration
        const unsigned long pumpDuration = 1000; // 1 second
        ActuatorController::runActuator("basePump", flowRate, pumpDuration);
        
        lastAdjustmentTime = millis();
        
        Logger::log(LogLevel::INFO, "pH PID update - Setpoint: " + String(phSetpoint) + 
                    ", Input: " + String(phInput) + ", Output: " + String(flowRate) + 
                    ", Duration: " + String(pumpDuration));
    } else if (abs(phInput - phSetpoint) <= phHysteresis) {
        ActuatorController::stopActuator("basePump");  // Just stop pump but keep PID active
        Logger::log(LogLevel::INFO, "pH within hysteresis range (" +
                    String(phHysteresis) + "). Base dosing paused.");
    }
}

void PIDManager::updateDOPID() {
    if (!doPIDRunning) return;

    doInput = SensorController::readSensor("oxygenSensor");
    doPID.Compute();

    if (abs(doInput - doSetpoint) > doHysteresis) {
        ActuatorController::runActuator("airPump", doOutput, 0);  // 0 pour une durée continue   
        Logger::log(LogLevel::INFO, "DO PID update - Setpoint: " + String(doSetpoint) + ", Input: " + String(doInput) + ", Output: " + String(doOutput));
    /*} else if (doInput >= doSetpoint) {
        // If the O2 level is sufficient, reduce the pump speed to minimum
        ActuatorController::runActuator("airPump", 10, 0); // 10% as minimum speed
        Logger::log(LogLevel::INFO, F("DO sufficient. Running air pump at minimum speed."));*/
    } else {
        ActuatorController::stopActuator("airPump");
        // ActuatorController::runActuator("airPump", 20, 0);  // Maintain minimum aeration
        Logger::log(LogLevel::INFO, "DO within hysteresis range (" +
                    String(doHysteresis) + "). Aeration paused.");
    }
}

void PIDManager::stopTemperaturePID() {
    tempPIDRunning = false;
    tempOutput = 0;
    ActuatorController::stopActuator("heatingPlate");
    Logger::log(LogLevel::INFO, F("Temperature PID stopped"));
}

void PIDManager::stopPHPID() {
    phPIDRunning = false;
    phOutput = 0;
    ActuatorController::stopActuator("basePump");
    //Logger::log(LogLevel::INFO, "pH PID stopped");
    Logger::log(LogLevel::INFO, F("pH PID stopped"));
}

void PIDManager::stopDOPID() {
    doPIDRunning = false;
    doOutput = 0;
    ActuatorController::stopActuator("airPump");
    //Logger::log(LogLevel::INFO, "DO PID stopped");
    Logger::log(LogLevel::INFO, F("DO PID stopped"));
}

//void PIDManager::stopAllPID() {
void PIDManager::stop() {
    stopTemperaturePID();
    stopPHPID();
    stopDOPID();
    //Logger::log(LogLevel::INFO, "All PID controls stopped");
    Logger::log(LogLevel::INFO, F("All PID controls stopped"));
}

void PIDManager::pauseAllPID() {
    tempPID.SetMode(MANUAL);
    phPID.SetMode(MANUAL);
    doPID.SetMode(MANUAL);
}

void PIDManager::resumeAllPID() {
    tempPID.SetMode(AUTOMATIC);
    phPID.SetMode(AUTOMATIC);
    doPID.SetMode(AUTOMATIC);
}

void PIDManager::adjustPIDParameters(const String& pidType, double Kp, double Ki, double Kd) {
    if (pidType == "temperature") {
        tempPID.SetTunings(Kp, Ki, Kd);
    } else if (pidType == "pH") {
        phPID.SetTunings(Kp, Ki, Kd);
    } else if (pidType == "DO") {
        doPID.SetTunings(Kp, Ki, Kd);
    }
}

// A sauvegarder/charger sur le serveur SI BESOIN de plus de data.
void PIDManager::saveParameters(const char* filename) {
    // Implement saving PID parameters to EEPROM or SD card
    Logger::log(LogLevel::INFO, "Saving PID parameters to " + String(filename));
}
void PIDManager::loadParameters(const char* filename) {
    // Implement loading PID parameters from EEPROM or SD card
    Logger::log(LogLevel::INFO, "Loading PID parameters from " + String(filename));
}

double PIDManager::convertPIDOutputToFlowRate(double pidOutput) {
    double minFlowRate = ActuatorController::getPumpMinFlowRate("basePump");
    double maxFlowRate = ActuatorController::getPumpMaxFlowRate("basePump");
    return map(pidOutput, 0, 100, minFlowRate, maxFlowRate);
}
