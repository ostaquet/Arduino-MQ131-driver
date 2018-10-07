/******************************************************************************
 * Arduino-MQ131-driver                                                       *
 * --------------------                                                       *
 * Arduino driver for gas sensor MQ131 (O3, NOx and CL2)                      *
 * Author: Olivier Staquet                                                    *
 * Last version available on https://github.com/ostaquet/Arduino-MQ131-driver *
 ******************************************************************************/

#ifndef _MQ131_H_
#define _MQ131_H_

// Default values
#define TIME_TO_READ_MILLIS 240000
#define VALUE_R0 50235.30
#define TEMPERATURE_CELSIUS  20
#define HUMIDITY_PERCENT 65

class MQ131 {
	public:
		MQ131(int _pinPower, int _pinSensor);		// Initialize the driver
		
		bool begin();								// Manage a full cycle with delay()
													// without giving the hand back to
													// the main loop (delay() function included)

//		float readNOx();							// Read the concentration of gas
//		float readCL2();
//		float readO3();

		void setTimeToRead(long millis);			// Define the time to read after
													// started the heater
		long getTimeToRead();						// Obtain time to read
													// (set automatically by calibrate() function)

//		void setR0(float valueR0);					// Define the R0 for the calibration
//		float setR0();								// Obtain the R0 currently set
													// (set automatically by calibrate() function)

//		void setEnv(int tempCels, int humPc);		// Define the temperature (in Celsius)
													// and humidity (in %) to adjust the
													// output values based on typical
													// characteristics of the MQ131

//		void calibrate();							// Run a calibration cycle
													// Ideally, 20°C 65% humidity in
													// clean fresh air (can take some minutes)

	protected:
		void startHeater();
		bool isTimeToRead();
		void stopHeater();

	private:
		int pinPower = -1;
		int pinSensor = -1;
		long millisLastStart = -1;
		long millisToRead = TIME_TO_READ_MILLIS;
		float valueR0 = VALUE_R0;
		float lastValueRs = -1;
		int temperatureCelsuis = TEMPERATURE_CELSIUS;
		int humidityPercent = HUMIDITY_PERCENT;
}

#endif // _MQ131_H_