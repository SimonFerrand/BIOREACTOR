#ifndef LOGGER_H
#define LOGGER_H

#include "DCPump.h"
#include "PeristalticPump.h"
#include "StirringMotor.h"
#include "HeatingPlate.h"
#include "LEDGrowLight.h"
#include "PT100Sensor.h"
#include "DS18B20TemperatureSensor.h"
#include "PHSensor.h"
#include "TurbiditySensor.h"
#include "OxygenSensor.h"
#include "AirFlowSensor.h"
#include <SoftwareSerial.h> // Include SoftwareSerial

extern SoftwareSerial espSerial; // Declare espSerial as an external variable

void logData(DCPump& airPump, DCPump& drainPump, PeristalticPump& nutrientPump, PeristalticPump& basePump,
    StirringMotor& stirringMotor, HeatingPlate& heatingPlate, LEDGrowLight& ledGrowLight,
    PT100Sensor& waterTempSensor, DS18B20TemperatureSensor& airTempSensor, PHSensor& phSensor,
    TurbiditySensor& turbiditySensor, OxygenSensor& oxygenSensor, AirFlowSensor& airFlowSensor,
    const String& currentProgram, const String& programStatus);

void logStartupParameters(const String& programType, int rateOrSpeed, int duration,
    float tempSetpoint, float phSetpoint, float doSetpoint, float nutrientConc,
    float baseConc, const String& experimentName, const String& comment);

#endif
