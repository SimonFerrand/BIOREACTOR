#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>
#include <ArduinoJson.h>

class Communication {
public:
    Communication(HardwareSerial& serial);
    void begin(unsigned long baud);
    bool available();
    String readMessage();
    void sendMessage(const String& message);
    void processCommand(const String& command);

private:
    HardwareSerial& _serial;
    static const unsigned int MAX_MESSAGE_LENGTH = 256;
};

#endif