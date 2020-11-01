/******************************************************************************
 * Arduino-MQ131-driver                                                       *
 * --------------------                                                       *
 * Arduino driver for gas sensor MQ131 (O3)                                   *
 * Author: Olivier Staquet                                                    *
 * Last version available on https://github.com/ostaquet/Arduino-MQ131-driver *
 ******************************************************************************
 * MIT License
 *
 * Copyright (c) 2018 Olivier Staquet
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *******************************************************************************/

#ifndef _MQ131_H_
#define _MQ131_H_

#include <Arduino.h>

// Default values
#define MQ131_DEFAULT_RL                            1000000           // Default load resistance of 1MOhms
#define MQ131_DEFAULT_STABLE_CYCLE                  15                // Number of cycles with low deviation to consider
                                                                      // the calibration as stable and reliable
#define MQ131_DEFAULT_TEMPERATURE_CELSIUS           20                // Default temperature to correct environmental drift
#define MQ131_DEFAULT_HUMIDITY_PERCENT              65                // Default humidity to correct environmental drift
#define MQ131_DEFAULT_LO_CONCENTRATION_R0           1917.22           // Default R0 for low concentration MQ131
#define MQ131_DEFAULT_LO_CONCENTRATION_TIME2READ    80                // Default time to read before stable signal for low concentration MQ131
#define MQ131_DEFAULT_HI_CONCENTRATION_R0           235.00            // Default R0 for high concentration MQ131
#define MQ131_DEFAULT_HI_CONCENTRATION_TIME2READ    80                // Default time to read before stable signal for high concentration MQ131

enum MQ131Model {LOW_CONCENTRATION, HIGH_CONCENTRATION,SN_O2_LOW_CONCENTRATION};
enum MQ131Unit {PPM, PPB, MG_M3, UG_M3};

class MQ131Class {
	public:
    // Constructor
    MQ131Class(uint32_t _RL);
    virtual ~MQ131Class();
  
		// Initialize the driver
		void begin(uint8_t _pinPower, uint8_t _pinSensor, MQ131Model _model, uint32_t _RL, Stream* _debugStream = NULL);

		// Manage a full cycle with delay() without giving the hand back to
		// the main loop (delay() function included)
		void sample();								

		// Read the concentration of gas
		// The environment should be set for accurate results
		float getO3(MQ131Unit unit);

		// Define environment
		// Define the temperature (in Celsius) and humidity (in %) to adjust the
		// output values based on typical characteristics of the MQ131
		void setEnv(int8_t tempCels, uint8_t humPc);

		// Setup calibration: Time to read
		// Define the time to read after started the heater
		// Get function also available to know the value after calibrate()
		// (the time to read is calculated automatically after calibration)
		void setTimeToRead(uint32_t sec);
		long getTimeToRead();

		// Setup calibration: R0
		// Define the R0 for the calibration
		// Get function also available to know the value after calibrate()
		// (the time to read is calculated automatically after calibration)
		void setR0(float _valueR0);
		float getR0();

		// Launch full calibration cycle
		// Ideally, 20°C 65% humidity in clean fresh air (can take some minutes)
		// For further use of calibration values, please use getTimeToRead() and getR0()
		void calibrate();

	private:
    		// Internal helpers
		// Internal function to manage the heater
		void startHeater();
		bool isTimeToRead();
		void stopHeater();

		// Internal reading function of Rs
		float readRs();

		// Get environmental correction to apply on ration Rs/R0
		float getEnvCorrectRatio();

    		// Convert gas unit of gas concentration
    		float convert(float input, MQ131Unit unitIn, MQ131Unit unitOut);

    		// Internal variables
		// Model of MQ131
		MQ131Model model;

    		// Serial console for the debug
    		Stream* debugStream = NULL;
    		bool enableDebug = false;

		// Details about the circuit: pins and load resistance value
		uint8_t pinPower = -1;
		uint8_t pinSensor = -1;
		uint32_t valueRL = -1;

		// Timer to keep track of the pre-heating
		uint32_t secLastStart = -1;
		uint32_t secToRead = -1;

		// Calibration of R0
		float valueR0 = -1;

		// Last value for sensor resistance
		float lastValueRs = -1;

		// Parameters for environment
		int8_t temperatureCelsuis = MQ131_DEFAULT_TEMPERATURE_CELSIUS;
		uint8_t humidityPercent = MQ131_DEFAULT_HUMIDITY_PERCENT;
};

extern MQ131Class MQ131;

#endif // _MQ131_H_
