/******************************************************************************
 * Arduino-MQ131-driver                                                       *
 * --------------------                                                       *
 * Arduino driver for gas sensor MQ131 (O3, NOx and CL2)                      *
 * Author: Olivier Staquet                                                    *
 * Last version available on https://github.com/ostaquet/Arduino-MQ131-driver *
 ******************************************************************************/

#ifndef _MQ131_H_
#define _MQ131_H_

#include <Arduino.h>

// Default values
#define TIME_TO_READ_MILLIS 150000
#define VALUE_R0_METAL 417.09
#define TEMPERATURE_CELSIUS  20
#define HUMIDITY_PERCENT 65
#define RL_OHMS 10000

enum MQ131Model {BLACK_BAKELITE, METAL};

class MQ131 {
	public:
		// Initialize the driver (eventually with RL value in Ohms)
		MQ131(int _pinPower, int _pinSensor, MQ131Model _model);
		MQ131(int _pinPower, int _pinSensor, MQ131Model _model, int _RL);

		// Manage a full cycle with delay() without giving the hand back to
		// the main loop (delay() function included)
		void begin();								

		// Read the concentration of gas (return value in ppb - Parts Per Billion)
		// The environment should be set for accurate results
		float readNOx();
		float readCL2();
		float readO3();

		// Define environment
		// Define the temperature (in Celsius) and humidity (in %) to adjust the
		// output values based on typical characteristics of the MQ131
		void setEnv(int tempCels, int humPc);

		// Setup calibration: Time to read
		// Define the time to read after started the heater
		// Get function also available to know the value after calibrate()
		// (the time to read is calculated automatically after calibration)
		void setTimeToRead(long millis);
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
//		void calibrate();

	protected:
		// Internal function to manage the heater
		void startHeater();
		bool isTimeToRead();
		void stopHeater();

		// Internal reading function of Rs
		float readRs();

		// Get environmental correction to apply on ration Rs/R0
		float getEnvCorrectRatio();

	private:
		// Model of MQ131
		MQ131Model model;

		// Details about the circuit: pins and load resistance value
		int pinPower = -1;
		int pinSensor = -1;
		int valueRL = RL_OHMS;

		// Timer to keep track of the pre-heating
		long millisLastStart = -1;
		long millisToRead = TIME_TO_READ_MILLIS;

		// Calibration of R0
		float valueR0 = VALUE_R0_METAL;

		// Last value for sensor resistance
		float lastValueRs = -1;

		// Parameters for environment
		int temperatureCelsuis = TEMPERATURE_CELSIUS;
		int humidityPercent = HUMIDITY_PERCENT;
};

#endif // _MQ131_H_