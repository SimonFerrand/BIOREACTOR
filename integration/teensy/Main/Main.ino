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
#include "QuadChannelDACController.h"

#include "TestsProgram.h"
#include "DrainProgram.h"
#include "MixProgram.h"
#include "FermentationProgram.h"

// WARNING, pins 2 and 3 do not work well in this system. Avoid allocating them without double-checking.

// Define serial port for communication 
#define SerialESP Serial7 // (RX = 28, TX=29)
#define SerialTurbidity Serial8 // (RX: Blue = 34, TX: green = 35) 
#define SerialSensoTransmitter Serial3 // For communication with SensoTransmitter (pH & O2 sensor) on Arduino Uno
  // DAC (Digital to Analogic Converter) - MCP4728
#define QUAD_CHANNEL_DAC_SDA_PIN 25
#define QUAD_CHANNEL_DAC_SCL_PIN 24
#define QUAD_CHANNEL_DAC_ADDRESS 0x60 // // Define the I2C address for the new QuadChannelDAC. Please note that for this MCP4728 DAC, I was unable to change the address via software. The address is locked at 0x60. If another MCP4728 DAC is to be used, it will have to be connected to another I2C.

// Sensor declarations
PT100Sensor waterTempSensor(10, 11, 12, 13, "waterTempSensor");  // Water temperature sensor (CS: 10, DI: 11, DO: 12, CLK: 13)
DS18B20TemperatureSensor airTempSensor(39, "airTempSensor");     // Air temperature sensor (Data: 52)
DS18B20TemperatureSensor electronicTempSensor(36, "electronicTempSensor");     // Electronic temperature sensor (Data: 29)
PHSensor phSensor(&SerialSensoTransmitter, &waterTempSensor, "phSensor");             // pH sensor (Analog: A1, uses water temp for compensation)
OxygenSensor oxygenSensor(&SerialSensoTransmitter, &waterTempSensor, "oxygenSensor"); // Dissolved oxygen sensor (Analog: A3, uses water temp)
AirFlowSensor airFlowSensor(23, "airFlowSensor");                // Air flow sensor (Digital: 37)
TurbiditySensorSEN0554 turbiditySensorSEN0554(&SerialTurbidity, "turbiditySensorSEN0554"); // SEN0554 turbidity sensor (RX: Blue, TX: green)
//TurbiditySensor turbiditySensor(A2, "turbiditySensor");          // Turbidity sensor (Analog: A2) 

// Actuator declarations
DCPump airPump(MCP4728_CHANNEL_A, 6, 30, "airPump");        // Air pump (PWM: MCP4728_CHANNEL_A, Relay: 6, Min PWM: 15) - 12V
DCPump drainPump(MCP4728_CHANNEL_B, 9, 30, "drainPump");    // Drain pump (PWM: MCP4728_CHANNEL_B, Relay: 29, Min PWM: 15) - 24V         //3  >9
DCPump samplePump(MCP4728_CHANNEL_C, 8, 30, "samplePump");// Sample pump (PWM: MCP4728_CHANNEL_C, Relay: 28, Min PWM: 15) - 24V          //2  > 8
DCPump fillPump(MCP4728_CHANNEL_D, 5, 30, "fillPump");// Fill pump (PWM: MCP4728_CHANNEL_D, Relay: 7, Min PWM: 15) - 24V
PeristalticPump nutrientPump(0x61, 0, 3, 105.0, "nutrientPump"); // Nutrient pump (I2C: 0x61, Relay: 0, Min flow: 1, Max flow: 105.0) - 24V ; Allocate IC2 address by soldering A0 input to Vcc on MCP4725 board 
PeristalticPump basePump(0x60, 1, 3, 105.0, "basePump");         // Base pump (I2C: 0x60, Relay: 8, Min flow: 1, Max flow: 105.0) ; NaOH @10% - 24V 
StirringMotor stirringMotor(22, 7, 390, 1500,"stirringMotor");   // Stirring motor (PWM: 9, Relay: 10, Min RPM: 390, Max RPM: 1000) - 12V
HeatingPlate heatingPlate(4, false, "heatingPlate");            // Heating plate (Relay: 12, Not PWM capable) - 24V
LEDGrowLight ledGrowLight(32, "ledGrowLight");                   // LED grow light (Relay: 27) 

// System components
VolumeManager volumeManager(0.85, 0.95, 0.4); // (totalVolume, maxVolumePercent, minVolume)
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

    // Initialize the QuadChannelDACController for the DC pump
        // Initialize I2C2 for QuadChannelDAC
    Wire2.setSDA(QUAD_CHANNEL_DAC_SDA_PIN);
    Wire2.setSCL(QUAD_CHANNEL_DAC_SCL_PIN);
    Wire2.begin();
        // Initialize the QuadChannelDACController with the specified address
    QuadChannelDACController::getInstance().begin(QUAD_CHANNEL_DAC_ADDRESS, QUAD_CHANNEL_DAC_SDA_PIN, QUAD_CHANNEL_DAC_SCL_PIN);
  
    // Initialize serial communication
    Serial.begin(115200);  // Initialize serial communication for debugging
    espCommunication.begin(9600); // Initialize serial communication with ESP32
    SerialTurbidity.begin(9600); // // Initialize serial communication with turbiditySensorSEN0554
    SerialSensoTransmitter.begin(9600); // Initialize communication with the pH and O2 transmitter on Arduino Uno
    
    // Initialize dataCollector
    Logger::initialize(dataCollector);
    //Logger::log(LogLevel::INFO, "Setup started");
    Logger::log(LogLevel::INFO, F("Setup started"));

        // Initialize actuators
    ActuatorController::initialize(airPump, drainPump, nutrientPump, basePump,
                                   stirringMotor, heatingPlate, ledGrowLight, samplePump, fillPump);
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
        /*
        Kp (Proportional) : Reacts proportionally to the current error
        Ki (Integral) = Considers the accumulation of past errors
        Kd (Derivative) = Reacts to the rate of change of the error
        =>To obtain a more aggressive response: First increase Kp to obtain a stronger reaction, Increase Ki if the error persists too long, Adjust Kd to avoid excessive oscillations. 
        */
    pidManager.initialize(2.0, 5.0, 1.0,  // (tempKp, tempKi, tempKd)
                          2.0, 3.0, 1.0,  // (phKp, phKi, phKd,)
                          15.0, 8.0, 2.0); // (doKp, doKi, doKd) // 10.0, 5.0, 2.0
    pidManager.setHysteresis(0.5, 0.2, 0.3); // Température Hystérésis : 0.5 à 1.0 °C; pH Hystérésis : 0.2 à 0.3; Oxygène Dissous Hystérésis : 0.2 g/L
    //Logger::log(LogLevel::INFO, "PID setup");
    Logger::log(LogLevel::INFO, F("PID setup"));

    volumeManager.setInitialVolume(0.5);           // set an initial volume of 0.2 L     // 750
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


