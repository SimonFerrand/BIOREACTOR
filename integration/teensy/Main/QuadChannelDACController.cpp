// Development for DAC "MCP4728"

// QuadChannelDACController.cpp
#include "QuadChannelDACController.h"
#include "Logger.h"

QuadChannelDACController* QuadChannelDACController::instance = nullptr;

QuadChannelDACController& QuadChannelDACController::getInstance() {
    if (instance == nullptr) {
        instance = new QuadChannelDACController();
    }
    return *instance;
}

QuadChannelDACController::QuadChannelDACController() : _wire(&Wire2) {}

void QuadChannelDACController::begin(uint8_t address, int sdaPin, int sclPin) {
    _address = address;
    _sdaPin = sdaPin;
    _sclPin = sclPin;

    // Configure I2C2 pins
    _wire->setSDA(_sdaPin);
    _wire->setSCL(_sclPin);
    _wire->begin();

    // Initialize the DAC with the specified address
    if (!_dac.begin(_address, _wire)) {
        Logger::log(LogLevel::ERROR, F("Failed to initialize Quad Channel DAC"));
        return;
    }

    Logger::log(LogLevel::INFO, "Quad Channel DAC initialized on I2C2 with address 0x" + String(_address, HEX));
}

void QuadChannelDACController::setVoltage(uint8_t channel, uint16_t voltage) {
    _dac.setChannelValue(static_cast<MCP4728_channel_t>(channel), voltage);
}