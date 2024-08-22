/*
 * Bioreactor Control System
 * 
 * This program controls a bioreactor system, managing various sensors and actuators.
 * It communicates with an ESP32 module via serial communication to receive commands
 * and send sensor data. The system can perform different operations like mixing,
 * draining, fermentation, and individual PID control based on received commands.
 * 
 * Key components:
 * - Sensors: pH, oxygen, turbidity, temperature (water and air), air flow
 * - Actuators: pumps (air, drain, nutrient, base), stirring motor, heating plate, LED grow light
 * - Communication: Serial interface with ESP32
 * - State Machine: Manages the overall state and operations of the bioreactor
 * 
 * The program continuously checks for incoming commands, updates the state machine,
 * and logs sensor data at regular intervals.
 */

// main.ino
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#include "SensorController.h"
#include "ActuatorController.h"
#include "StateMachine.h"
#include "VolumeManager.h"
#include "SafetySystem.h"
#include "Logger.h"
#include "PIDManager.h"
#include "CommandHandler.h"
#include "Communication.h"
#include "DataCollector.h"

#include "TestsProgram.h"
#include "DrainProgram.h"
#include "MixProgram.h"
#include "FermentationProgram.h"

// Define serial port for communication with ESP32
#define SerialESP Serial1

// Sensor declarations
PT100Sensor waterTempSensor(22, 23, 24, 25, "waterTempSensor");  // Water temperature sensor (CS: 22, DI: 23, DO: 24, CLK: 25)
DS18B20TemperatureSensor airTempSensor(52, "airTempSensor");     // Air temperature sensor (Data: 52)
DS18B20TemperatureSensor electronicTempSensor(29, "electronicTempSensor");     // Electronic temperature sensor (Data: 29)
PHSensor phSensor(A1, &waterTempSensor, "phSensor");             // pH sensor (Analog: A1, uses water temp for compensation)
//TurbiditySensor turbiditySensor(A2, "turbiditySensor");          // Turbidity sensor (Analog: A2)
OxygenSensor oxygenSensor(A3, &waterTempSensor, "oxygenSensor"); // Dissolved oxygen sensor (Analog: A3, uses water temp)
AirFlowSensor airFlowSensor(26, "airFlowSensor");                // Air flow sensor (Digital: 26)
TurbiditySensorSEN0554 turbiditySensorSEN0554(A14, A15, "turbiditySensorSEN0554"); // SEN0554 turbidity sensor (RX: Blue, TX: green) 

// Actuator declarations
DCPump airPump(5, 6, 10, "airPump");        // Air pump (PWM: 5, Relay: 6, Min PWM: 10)
DCPump drainPump(4, 30, 15, "drainPump");    // Drain pump (PWM: 4, Relay: 29, Min PWM: 15)
DCPump samplePump(3, 28, 15, "samplePump");// Sample pump (PWM: 3, Relay: 28, Min PWM: 15)
PeristalticPump nutrientPump(0x61, 7, 1, 105.0, "nutrientPump"); // Nutrient pump (I2C: 0x61, Relay: 7, Min flow: 1, Max flow: 105.0)
PeristalticPump basePump(0x60, 8, 1, 105.0, "basePump");         // Base pump (I2C: 0x60, Relay: 8, Min flow: 1, Max flow: 105.0)
StirringMotor stirringMotor(9, 10, 390, 1000,"stirringMotor");   // Stirring motor (PWM: 9, Relay: 10, Min RPM: 390, Max RPM: 1000)
HeatingPlate heatingPlate(12, false, "heatingPlate");            // Heating plate (Relay: 12, Not PWM capable)
LEDGrowLight ledGrowLight(27, "ledGrowLight");                   // LED grow light (Relay: 27)

// System components
VolumeManager volumeManager(0.6, 0.95, 0.1);
DataCollector dataCollector(volumeManager);
Communication espCommunication(SerialESP, dataCollector);
PIDManager pidManager;
SafetySystem safetySystem(1.0, 0.95, 0.1);
StateMachine stateMachine(pidManager, volumeManager, espCommunication);

// Program declarations
TestsProgram testsProgram(pidManager);
DrainProgram drainProgram;
MixProgram mixProgram;
FermentationProgram fermentationProgram(pidManager, volumeManager);

CommandHandler commandHandler(stateMachine, safetySystem, volumeManager, pidManager);

unsigned long previousMillis = 0;
const long interval = 30000; // Interval for logging (30 seconds)

void setup() {
    Serial.begin(115200);  // Initialize serial communication for debugging
    espCommunication.begin(9600); // Initialize serial communication with ESP32

    Logger::initialize(dataCollector);
    //Logger::log(LogLevel::INFO, "Setup started");
    Logger::log(LogLevel::INFO, F("Setup started"));
    

    // Initialize sensors
    SensorController::initialize(waterTempSensor, airTempSensor, electronicTempSensor,
                                 phSensor,
                                 oxygenSensor, 
                                 airFlowSensor, 
                                 turbiditySensorSEN0554);
    SensorController::beginAll();

    // Initialize actuators
    ActuatorController::initialize(airPump, drainPump, nutrientPump, basePump,
                                   stirringMotor, heatingPlate, ledGrowLight, samplePump);
    ActuatorController::beginAll();
    

    // Add programs to the state machine
    stateMachine.addProgram("Tests", &testsProgram);
    stateMachine.addProgram("Drain", &drainProgram);
    stateMachine.addProgram("Mix", &mixProgram);
    stateMachine.addProgram("Fermentation", &fermentationProgram);

    // Initialisation of the PIDManager to define hysteresis values
    pidManager.initialize(2.0, 5.0, 1.0, 2.0, 5.0, 1.0, 2.0, 5.0, 1.0);
    pidManager.setHysteresis(0.5, 0.05, 1.0);
    //Logger::log(LogLevel::INFO, "PID setup");
    Logger::log(LogLevel::INFO, F("PID setup"));

    volumeManager.setInitialVolume(0.3);           // set an initial volume of 0.2 L
    //Logger::log(LogLevel::INFO, "Setup an initial volume");

    //Logger::log(LogLevel::INFO, "Setup completed");
    Logger::log(LogLevel::INFO, F("Setup completed"));
}

void loop() {
    // Check for incoming commands from ESP32
    if (espCommunication.available()) {
        String receivedData = espCommunication.readMessage();
        if (receivedData.length() > 0) {
            Logger::log(LogLevel::INFO, "Received from ESP32: " + receivedData);
            espCommunication.processCommand(receivedData);
        }
    }

    // Check for incoming commands from Arduino Serial Monitor
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        Logger::log(LogLevel::INFO, "Received from Serial Monitor: " + command);
        commandHandler.executeCommand(command);
    }
    
    // Update state machine
    stateMachine.update();

    // Update PID manager
    pidManager.updateAllPIDControllers();

    // Check safety limits
    //safetySystem.checkLimits();

    // Log data every interval
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        //Logger::logData(stateMachine.getCurrentProgram(), String(static_cast<int>(stateMachine.getCurrentState())));    
        //espCommunication.sendAllData(stateMachine.getCurrentProgram(), static_cast<int>(stateMachine.getCurrentState()));

        //String allData = dataCollector.collectAllData(stateMachine.getCurrentProgram(), static_cast<int>(stateMachine.getCurrentState()));
        //Logger::logAllData(allData);
        //espCommunication.sendAllData(stateMachine.getCurrentProgram(), static_cast<int>(stateMachine.getCurrentState()));

        Logger::logAllData(stateMachine.getCurrentProgram(), static_cast<int>(stateMachine.getCurrentState()));
        espCommunication.sendAllData(stateMachine.getCurrentProgram(), static_cast<int>(stateMachine.getCurrentState()));
        
    }

    // Short pause to avoid excessive CPU usage
    delay(10);
}


