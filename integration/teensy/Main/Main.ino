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

// Define serial port for communication 
#define SerialESP Serial7 // (RX = 28, TX=29)
#define SerialTurbidity Serial8 // (RX: Blue = 34, TX: green = 35) 
#define SerialSensoTransmitter Serial3 // For communication with SensoTransmitter (pH & O2 sensor) on Arduino Uno

// Sensor declarations
PT100Sensor waterTempSensor(10, 11, 12, 13, "waterTempSensor");  // Water temperature sensor (CS: 10, DI: 11, DO: 12, CLK: 13)
DS18B20TemperatureSensor airTempSensor(39, "airTempSensor");     // Air temperature sensor (Data: 52)
DS18B20TemperatureSensor electronicTempSensor(20, "electronicTempSensor");     // Electronic temperature sensor (Data: 29)
PHSensor phSensor(&SerialSensoTransmitter, &waterTempSensor, "phSensor");             // pH sensor (Analog: A1, uses water temp for compensation)
//TurbiditySensor turbiditySensor(A2, "turbiditySensor");          // Turbidity sensor (Analog: A2)
OxygenSensor oxygenSensor(&SerialSensoTransmitter, &waterTempSensor, "oxygenSensor"); // Dissolved oxygen sensor (Analog: A3, uses water temp)
AirFlowSensor airFlowSensor(2, "airFlowSensor");                // Air flow sensor (Digital: 26)
TurbiditySensorSEN0554 turbiditySensorSEN0554(&SerialTurbidity, "turbiditySensorSEN0554"); // SEN0554 turbidity sensor (RX: Blue, TX: green) 

// Actuator declarations
DCPump airPump(23, 6, 10, "airPump");        // Air pump (PWM: 5, Relay: 6, Min PWM: 10) - 12V
DCPump drainPump(41, 3, 15, "drainPump");    // Drain pump (PWM: 4, Relay: 29, Min PWM: 15) - 24V
DCPump samplePump(7, 2, 15, "samplePump");// Sample pump (PWM: 3, Relay: 28, Min PWM: 15) - 24V
PeristalticPump nutrientPump(0x61, 0, 1, 105.0, "nutrientPump"); // Nutrient pump (I2C: 0x61, Relay: 7, Min flow: 1, Max flow: 105.0) - 24V ; Allocate IC2 address by soldering A0 input to Vcc on MCP4725 board 
PeristalticPump basePump(0x60, 1, 1, 105.0, "basePump");         // Base pump (I2C: 0x60, Relay: 8, Min flow: 1, Max flow: 105.0) ; NaOH @10% - 24V 
StirringMotor stirringMotor(16, 5, 390, 550,"stirringMotor");   // Stirring motor (PWM: 9, Relay: 10, Min RPM: 390, Max RPM: 1000) - 12V
HeatingPlate heatingPlate(4, false, "heatingPlate");            // Heating plate (Relay: 12, Not PWM capable) - 24V
LEDGrowLight ledGrowLight(32, "ledGrowLight");                   // LED grow light (Relay: 27) 

// System components
VolumeManager volumeManager(0.75, 0.95, 0.1);
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
const long measurement_interval = 15000; // Interval for logging (30 seconds)

void setup() {

      // Initialize actuators
    ActuatorController::initialize(airPump, drainPump, nutrientPump, basePump,
                                   stirringMotor, heatingPlate, ledGrowLight, samplePump);
    ActuatorController::beginAll();
    
    Serial.begin(115200);  // Initialize serial communication for debugging
    espCommunication.begin(9600); // Initialize serial communication with ESP32
    SerialTurbidity.begin(9600); // // Initialize serial communication with turbiditySensorSEN0554
    SerialSensoTransmitter.begin(9600); // Initialize communication with the pH and O2 transmitter on Arduino Uno
    

    Logger::initialize(dataCollector);
    //Logger::log(LogLevel::INFO, "Setup started");
    Logger::log(LogLevel::INFO, F("Setup started"));
    
    // Initialize actuators
    ActuatorController::initialize(airPump, drainPump, nutrientPump, basePump,
                                   stirringMotor, heatingPlate, ledGrowLight, samplePump);
    ActuatorController::beginAll();

    // Initialize sensors
    SensorController::initialize(waterTempSensor, airTempSensor, electronicTempSensor,
                                 phSensor,
                                 oxygenSensor, 
                                 airFlowSensor, 
                                 turbiditySensorSEN0554);
    SensorController::beginAll();

    // Add programs to the state machine
    stateMachine.addProgram("Tests", &testsProgram);
    stateMachine.addProgram("Drain", &drainProgram);
    stateMachine.addProgram("Mix", &mixProgram);
    stateMachine.addProgram("Fermentation", &fermentationProgram);

    // Initialisation of the PIDManager to define hysteresis values
    pidManager.initialize(2.0, 5.0, 1.0, 2.0, 5.0, 1.0, 2.0, 5.0, 1.0);
    pidManager.setHysteresis(0.8, 0.25, 2.5); // Température Hystérésis : 0.5 à 1.0 °C; pH Hystérésis : 0.2 à 0.3; Oxygène Dissous Hystérésis : 2.0 à 3.0 % sat
    //Logger::log(LogLevel::INFO, "PID setup");
    Logger::log(LogLevel::INFO, F("PID setup"));

    volumeManager.setInitialVolume(0.3);           // set an initial volume of 0.2 L
    //Logger::log(LogLevel::INFO, "Setup an initial volume");

    //Logger::log(LogLevel::INFO, "Setup completed");
    Logger::log(LogLevel::INFO, F("Setup completed"));
    Serial.println();

        // Ajouter ces lignes pour lancer l'air pump et le stirring motor à 100%
    //ActuatorController::runActuator("airPump", 50, 0);  // 100% speed, 0 duration (continuous)
    //ActuatorController::runActuator("stirringMotor", 500, 0);  // Max RPM, 0 duration (continuous)

    

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
    if (currentMillis - previousMillis >= measurement_interval) {
        previousMillis = currentMillis;

        // Take a fresh sample
        //SensorController::takeSample();

        // log all data
        Logger::logAllData(stateMachine.getCurrentProgram(), static_cast<int>(stateMachine.getCurrentState()));

        // send all data to server
        espCommunication.sendAllData(stateMachine.getCurrentProgram(), static_cast<int>(stateMachine.getCurrentState()));
        
    }

    // Short pause to avoid excessive CPU usage
    delay(10);
}

