/*
Follow "Probe Calibration" part on @https://wiki.dfrobot.com/Gravity__Analog_Dissolved_Oxygen_Sensor_SKU_SEN0237
*/

#include <Arduino.h>

#define VREF    5000//VREF(mv)
#define ADC_RES 1024//ADC Resolution

uint32_t raw;

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    raw=analogRead(A5); //select the pin
    Serial.println("raw:\t"+String(raw)+"\tVoltage(mv)"+String(raw*VREF/ADC_RES));
    delay(1000);
}