/*
 * TurbiditySensorSEN0554.h
 * This file defines a class for reading turbidity values from a UART non-contact optical turbidity sensor (SEN0554).
 * The class implements the SensorInterface and provides methods to initialize the sensor and read the turbidity value.
 *
 * Physical modules used:
 * - UART Non-contact Optical Turbidity Sensor SEN0554
 *
 * Installation Instructions:
 * 1. Connect the sensor to the Arduino board:
 *    - Black wire (GND) to GND
 *    - Red wire (VCC) to 5V
 *    - Blue wire (TX) to RX pin (e.g., pin 10 on Arduino)
 *    - Green wire (RX) to TX pin (e.g., pin 11 on Arduino)
 * 2. The two infrared probes should be oppositely installed on a transparent container with a diameter of 40-50mm.
 * 3. Ensure the liquid level is higher than the two probes for accurate measurements.
 *
 * Note: This sensor does not require calibration as it's pre-calibrated. However, ensure to use it within its specified operating temperature range (5-60°C).
 *
 * Reference: https://wiki.dfrobot.com/SKU_SEN0554_Turbidity_Sensor
 */

#ifndef TURBIDITYSENSORSEN0554_H
#define TURBIDITYSENSORSEN0554_H

#include "SensorInterface.h"
#include <SoftwareSerial.h>
#include <Arduino.h>

class TurbiditySensorSEN0554 : public SensorInterface {
public:
    TurbiditySensorSEN0554(int rxPin, int txPin, const char* name);  // Blue, green
    void begin() override;
    float readValue() override;
    const char* getName() const override { return _name; }

private:
    SoftwareSerial _serial;
    const char* _name;
    unsigned char _command[5] = {0x18, 0x05, 0x00, 0x01, 0x0D};
    unsigned char _response[5];
    const float SCALE_FACTOR = 1.5;
    const int OFFSET = 10;
    bool communicate();
};

#endif