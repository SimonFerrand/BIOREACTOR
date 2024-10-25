/*
 * OxygenSensor.h
 * This file defines a class for reading dissolved oxygen values from a dissolved oxygen sensor.
 * The class implements the SensorInterface and provides methods to initialize the sensor, read the DO value, and perform calibration.
 *
 * Physical modules used:
 * - Gravity: Analog Dissolved Oxygen Sensor / Meter Kit for Arduino
 */

/*
Installation Instructions:

For Gravity: Analog Dissolved Oxygen Sensor / Meter Kit for Arduino
@https://wiki.dfrobot.com/Gravity__Analog_Dissolved_Oxygen_Sensor_SKU_SEN0237
@https://www.instructables.com/Calibrated-Dissolved-Oxygen-Meter/

1. Connect the DO sensor to the Arduino board:
    - Signal Pin to an analog pin on the Arduino (A1 in this case).
    - V+ to 5V on the Arduino.
    - GND to GND on the Arduino.
    On the Arduino Mega, external interrupts are only available on certain specific pins. The pins that support interrupts are: 2, 3, 18, 19, 20, 21 (interrupts 0, 1, 5, 4, 3, 2 respectively).
2. Prepare the Probe
    For a new dissolved oxygen probe, 0.5 mol/L NaOH solution should be added into the membrane cap first as the filling solution. If the probe has been used for some time and the error grows greatly, it is time to change the filling solution. The following tutorial details how to fill the probe with the NaOH solution.
    - Unscrew the membrane cap from the probe and fill about 2/3 volume of the cap with 0.5 mol/L NaOH solution. Make sure the probe is in vertical position with respect to the horizontal plane. Carefully screw the cap back to the probe. It would be nice if a little bit solution overflows out of the cap to ensure the probe is fully filled with NaOH solution.
    - When screwing the cap back to the probe, the probe should be in vertical position with respect to the horizontal plane to avoid creating bubbles in the filling solution.
    - If the cap is fully filled with NaOH solution, there will be too much solution overflowing when screwing the cap back to the probe. If the filling solution is too little, bubbles may be created inside the cap. In sum, the best way is to fill about 2/3 volume of the cap. A little bit overflow when screwing the cap back to the probe is okay.
    - Clean the overflowed solution with tissue.
    - Screw the NaOH solution bottle after every use to prevent the CO2 in the air from affecting the solution.
3. Please ensure that the sensor is calibrated in air before the first use to ensure accurate readings. Submerge the sensor in water and wait several minutes for the readings to stabilize before calibration:
    -Use a stirrer or an eggbeater to continuously stir or Use an air pump to continuously inflate the water for 10 minutes to saturate the dissolved oxygen). To obtain two points, perform this procedure with one water cooled in the fridge and the other at room temperature or heated.
    -Add sodium sulfite(Na2SO3) or Thiosulfate Sodium (ex: 2g in 100 ml of water during 30+ min) into water until it is saturated. This can consume all the oxygen in the water to obtain the zero dissolved oxygen liquid.
4. For calibration in the air, expose the membrane to the air and set the output value to 100% oxygen saturation (can be done via software using calibration commands or physically via the potentiometer on the sensor board).
5. The actual DO measurement will vary with temperature. It is highly recommended to use a temperature compensation function which can be achieved by integrating a temperature sensor like the DS18B20 to provide real-time temperature data for more accurate DO readings.

Attention : It is necessary to measure the saturated dissolved oxygen voltage at two different temperatures to obtain a temperature compensation curve.

Protocols :
     - Filling solution : Start with ~20mL of water in a 50mL volumetric flask, SLOWLY add 3.3mL of 30.5% NaOH solution, fill with water up to 50mL mark, then close and gently mix by inversion several times.
     - 0 mg/L O2 calibration solution :   In a 50mL volumetric flask, dissolve 1g of Sodium Thiosulfate (Na2S2O3) in water, fill up to 50mL mark, mix gently until fully dissolved, then wait 1 hour to ensure complete oxygen removal before using for calibration.

Calibration protocol :
   1. Reset :
    o2 calibrate reset

  2. Calibration deux points :
    o2 calibrate start 2
    [Place probe in saturated water or zero solution]
    o2 calibrate save
    [Place probe in other solution]
    o2 calibrate save
    o2 calibrate finish
*/

#ifndef OXYGENSENSOR_H
#define OXYGENSENSOR_H

#include "SensorInterface.h"
#include <EEPROM.h>
#include <Arduino.h>

#define O2_EEPROM_ADDR 200

class OxygenSensor : public SensorInterface {
public:
    OxygenSensor(int pin, const char* name);
    void begin() override;
    float readValue(float temperature) override;
    const char* getName() const override { return _name; }

    // Points de calibration
    void saveZeroPoint(float temp);
    void saveSaturationLowTemp(float temp);
    void saveSaturationHighTemp(float temp);
    void resetCalibration();
    String getCalibrationStatus();



private:
    int _pin;
    const char* _name;

    // Configuration ADC
    static const uint16_t VREF = 5000;    // 5000mV
    static const uint16_t ADC_RES = 1024;  // ADC Resolution
    static const uint8_t SAMPLES_COUNT = 10;

    float zeroVoltage;
    float saturationVoltageLow;
    float saturationVoltageHigh;
    float zeroTemperature;
    float saturationTempLow;
    float saturationTempHigh;
    
    enum class CalibrationState {
        NONE,
        PARTIAL,
        COMPLETE
    };
    CalibrationState calibrationState;

    // Structure pour les données de calibration
    struct CalibrationData {
        float zeroVoltage;
        float saturationVoltageLow;
        float saturationVoltageHigh;
        float zeroTemperature;
        float saturationTempLow;
        float saturationTempHigh;
        int calibrationState;
        byte signature;  // Signature pour validation
    };

    // Limites de température
    static const uint8_t TEMP_MIN = 15;
    static const uint8_t TEMP_MAX = 40;
    static const float DO_TABLE[26];  // 15-40°C


    float readAverageVoltage();
    float calculateDO(float voltage, float temperature);
    float calculateUncalibratedDO(float voltage, float temperature);
    float getSaturationDO(float temperature);
    float interpolateVoltage(float temperature);
    void updateCalibrationState();
    void printDebugInfo(float voltage, float temperature, float doValue);
    void saveCalibrationToEEPROM();
    void loadCalibrationFromEEPROM();


};

#endif
