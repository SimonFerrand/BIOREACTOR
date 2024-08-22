// Communication.h
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "DataCollector.h"

class Communication {
public:
    Communication(HardwareSerial& serial, DataCollector& dataCollector);
    void begin(unsigned long baud);
    bool available();
    String readMessage();
    void sendMessage(const String& message);
    void processCommand(const String& command);
    void sendSensorData();
    void sendActuatorData();
    void sendVolumeData();
    void sendAllData(const String& currentProgram, int currentState);
    void sendProgramEvent(const String& programName, ProgramBase* program);
    
private:
    HardwareSerial& _serial;
    DataCollector& _dataCollector;
};

#endif // COMMUNICATION_H