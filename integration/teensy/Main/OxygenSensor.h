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
    -Add sodium sulfite(Na2SO3) into water until it is saturated. This can consume all the oxygen in the water to obtain the zero dissolved oxygen liquid.
4. For calibration in the air, expose the membrane to the air and set the output value to 100% oxygen saturation (can be done via software using calibration commands or physically via the potentiometer on the sensor board).
5. The actual DO measurement will vary with temperature. It is highly recommended to use a temperature compensation function which can be achieved by integrating a temperature sensor like the DS18B20 to provide real-time temperature data for more accurate DO readings.
*/

#ifndef OXYGENSENSOR_H
#define OXYGENSENSOR_H

#include "SensorInterface.h"
#include "PT100Sensor.h"
#include <EEPROM.h>
#include <Arduino.h>

#define PH_EEPROM_ADDR 0
#define O2_EEPROM_ADDR 100

class OxygenSensor : public SensorInterface {
public:
    OxygenSensor(int pin, PT100Sensor* tempSensor, const char* name);
    void begin() override;
    float readValue() override;
    const char* getName() const override { return _name; }

    void startCalibration(int points);
    void saveCalibrationPoint();
    void finishCalibration();
    String getCalibrationStatus() const;
    void resetCalibration();

private:
    int _pin;
    PT100Sensor* _tempSensor;
    const char* _name;

    static const uint16_t VREF = 5000;
    static const uint16_t ADC_RES = 1024;

    static const int MAX_CALIBRATION_POINTS = 3;
    int calibrationPoints;
    uint16_t calibrationVoltages[MAX_CALIBRATION_POINTS];
    uint8_t calibrationTemperatures[MAX_CALIBRATION_POINTS];

    enum class CalibrationState {
        NONE,
        IN_PROGRESS,
        COMPLETED
    };
    CalibrationState calibrationState;
    int currentCalibrationPoint;

    void saveCalibrationToEEPROM();
    void loadCalibrationFromEEPROM();

    static const uint16_t DO_Table[41];
};

#endif