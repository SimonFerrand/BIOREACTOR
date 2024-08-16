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
#include "PT100Sensor.h" // Include PT100Sensor for temperature compensation
#include <Arduino.h>

class OxygenSensor : public SensorInterface {
public:
    /*
     * Constructor for OxygenSensor.
     * @param pin: The analog pin connected to the DO sensor.
     * @param tempSensor: Pointer to a PT100 sensor object for temperature compensation.
     */
    OxygenSensor(int pin, PT100Sensor* tempSensor, const char* name);

    /*
     * Method to initialize the DO sensor.
     */
    void begin();

    /*
     * Method to read the DO value from the sensor.
     * @return: The DO concentration in mg/L.
     */
    float readValue() override;

    /*
     * Method for calibration
     */ 
    void calibrate(); 

    const char* getName() const override { return _name; }
    
private:
    int _pin;                       // Analog pin connected to the DO sensor
    PT100Sensor* _tempSensor;       // Pointer to the PT100 temperature sensor

    // Calibration constants
    static const uint16_t VREF = 5000; // Reference voltage (millivolts)
    static const uint16_t ADC_RES = 1024; // ADC resolution

    // Calibration mode selection: 0 for single-point, 1 for two-point
    static const uint8_t TWO_POINT_CALIBRATION = 1;

    // Single point calibration
    static const uint16_t CAL1_V = 1600; // Calibration voltage (millivolts)
    static const uint8_t CAL1_T = 25; // Calibration temperature (°C)

    // Two-point calibration
    static const uint16_t CAL2_V = 1300; // Calibration voltage (millivolts)
    static const uint8_t CAL2_T = 15; // Calibration temperature (°C)

    // Table of saturated dissolved oxygen levels for different temperatures
    static const uint16_t DO_Table[41];

    const char* _name;
};

#endif

