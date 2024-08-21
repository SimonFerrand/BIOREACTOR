// Communication.cpp
#include "Communication.h"
#include "CommandHandler.h"

extern CommandHandler commandHandler;

Communication::Communication(HardwareSerial& serial, DataCollector& dataCollector) 
    : _serial(serial), _dataCollector(dataCollector) {}

void Communication::begin(unsigned long baud) {
    _serial.begin(baud);
}

bool Communication::available() {
    return _serial.available() > 0;
}

String Communication::readMessage() {
    if (_serial.available() > 0) {
        String receivedData = _serial.readStringUntil('\n');
        receivedData.trim();
        if (receivedData.length() > 0) {
            return receivedData;
        }
    }
    return "";
}

void Communication::sendMessage(const String& message) {
    _serial.println(message);
}

void Communication::processCommand(const String& command) {
    if (command.length() == 0) {
        return;  // Do not process empty commands
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, command);

    if (error) {
        // If this is not a valid JSON, treat as a direct command
        commandHandler.executeCommand(command);
    } else {
        // This is a valid JSON, extract the information and create the command
        String program = doc["program"];
        
        if (program == "mix") {
            int speed = doc["speed"];
            String cmd = "mix " + String(speed);
            commandHandler.executeCommand(cmd);
        } else if (program == "drain") {
            int rate = doc["rate"];
            int duration = doc["duration"];
            String cmd = "drain " + String(rate) + " " + String(duration);
            commandHandler.executeCommand(cmd);
        } else if (program == "fermentation") {
            float tempSetpoint = doc["temperature"];
            float phSetpoint = doc["pH"];
            float doSetpoint = doc["dissolvedOxygen"];
            float nutrientConc = doc["nutrientConcentration"];
            float baseConc = doc["baseConcentration"];
            int duration = doc["duration"];
            String experimentName = doc["experimentName"];
            String comment = doc["comment"];
            String cmd = "fermentation " + String(tempSetpoint) + " " + String(phSetpoint) + " " +
                         String(doSetpoint) + " " + String(nutrientConc) + " " + String(baseConc) + " " +
                         String(duration) + " " + experimentName + " " + comment;
            commandHandler.executeCommand(cmd);
        } else if (program == "stop") {
            commandHandler.executeCommand("stop");
        } else {
            Serial.println("Unknown program: " + program);
        }
    }
}

void Communication::sendSensorData() {
    String sensorData = _dataCollector.collectSensorData();
    sendMessage(sensorData);
}

void Communication::sendActuatorData() {
    String actuatorData = _dataCollector.collectActuatorData();
    sendMessage(actuatorData);
}

void Communication::sendVolumeData() {
    String volumeData = _dataCollector.collectVolumeData();
    sendMessage(volumeData);
}