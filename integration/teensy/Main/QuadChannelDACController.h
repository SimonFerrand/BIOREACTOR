// Development for DAC "MCP4728"

// QuadChannelDACController.h
#ifndef QUAD_CHANNEL_DAC_CONTROLLER_H
#define QUAD_CHANNEL_DAC_CONTROLLER_H

#include <Wire.h>
#include <Adafruit_MCP4728.h>

class QuadChannelDACController {
public:
    static QuadChannelDACController& getInstance();
    void begin(uint8_t address, int sdaPin, int sclPin);
    void setVoltage(uint8_t channel, uint16_t voltage);

private:
    QuadChannelDACController();
    static QuadChannelDACController* instance;
    Adafruit_MCP4728 _dac;
    TwoWire* _wire;

    uint8_t _address;
    int _sdaPin;
    int _sclPin;
};

#endif