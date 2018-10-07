/******************************************************************************
 * Arduino-MQ131-driver                                                       *
 * --------------------                                                       *
 * Arduino driver for gas sensor MQ131 (O3, NOx and CL2)                      *
 * Author: Olivier Staquet                                                    *
 * Last version available on https://github.com/ostaquet/Arduino-MQ131-driver *
 ******************************************************************************/

 #include "MQ131.h"

/**
 * Init core variables
 */
 MQ131::MQ131(int _pinPower, int _pinSensor) {
 	// Store the pin info
 	pinPower = _pinPower;
 	pinSensor = _pinSensor;

 	// Setup pin mode
 	pinMode(pinPower, OUTPUT);
 	pinMode(pinSensor, INPUT);
 }

 MQ131::MQ131(int _pinPower, int _pinSensor, int _RL) {
 	// Store the pin info
 	pinPower = _pinPower;
 	pinSensor = _pinSensor;
 	valueRL = _RL;

 	// Setup pin mode
 	pinMode(pinPower, OUTPUT);
 	pinMode(pinSensor, INPUT);
 }

/**
 * Do a full cycle (heater, reading, stop heater)
 * The function gives back the hand only at the end
 * of the read cycle!
 */
 void MQ131::begin() {
 	startHeater();
 	while(!isTimeToRead()) {
 		delay(1000);
 	}
 	lastValueRs = readRs();
 	stopHeater();
 }

/**
 * Start the heater
 */
 void MQ131::startHeater() {
 	digitalWrite(pinPower, HIGH);
 	millisLastStart = millis();
 }

/**
 * Check if it is the right time to read the Rs value
 */
 bool MQ131::isTimeToRead() {
 	// Check if the heater has been started...
 	if(millisLastStart < 0) {
 		return false;
 	}
 	// OK, check if it's the time to read based on calibration parameters
 	if(millis() >= millisLastStart + getTimeToRead()) {
 		return true;
 	}
 	return false;
 } 

/**
 * Stop the heater
 */
 void MQ131::stopHeater() {
 	digitalWrite(pinPower, LOW);
 	millisLastStart = -1;
 }

/**
 * Get parameter time to read
 */
 long MQ131::getTimeToRead() {
 	return millisToRead;
 }

/**
 * Set parameter time to read (for calibration or to recall
 * calibration from previous run)
 */
 void MQ131::setTimeToRead(long millis) {
 	millisToRead = millis;
 }

/**
 * Read Rs value
 */
 float MQ131::readRs() {
 	// Read the value
 	int valueSensor = analogRead(pinSensor);
 	// Compute the voltage on load resistance (for 5V Arduino)
 	float vRL = ((float)valueSensor) / 1024.0 * 5.0;
 	// Compute the resistance of the sensor (for 5V Arduino)
 	float rS = (5.0 / vRL - 1.0) * valueRL;
 	return rS;
 }

/**
 * Set environmental values
 */
 void MQ131::setEnv(int tempCels, int humPc) {
 	temperatureCelsuis = tempCels;
 	humidityPercent = humPc;
 }

/**
 * Get correction to apply on Rs depending on environmental
 * conditions
 */
 float MQ131::getEnvCorrectRatio() {
 	// Select the right equation based on humidity
 	// For humidity > 75%, use the 85% curve
 	if(humidityPercent > 75) {
 		return -0.0141 * temperatureCelsuis + 1.5623;
 	}
 	// For humidity > 50%, use the 60% curve
 	if(humidityPercent > 50) {
 		return -0.0119 * temperatureCelsuis + 1.3261;
 	}

 	// Humidity < 50%, use the 30% curve
 	return -0.0103 * temperatureCelsuis + 1.1507;
 }

/**
 * Get gas concentration for NOx in ppm
 */
 float MQ131::readNOx() {
 	// If no value Rs read, return 0.0
 	if(lastValueRs < 0) {
 		return 0.0;
 	}

 	// Compute the ratio Rs/R0 and apply the environmental correction
 	float ratio = lastValueRs / valueR0 * getEnvCorrectRatio();

 	// Use the equation to compute the NOx concentration in ppm
 	// R^2 = 0.997
 	float ppm = 456.23 * pow(ratio, -2.204);
 	return ppm;
 }

 /**
 * Get gas concentration for CL2 in ppm
 */
 float MQ131::readCL2() {
 	// If no value Rs read, return 0.0
 	if(lastValueRs < 0) {
 		return 0.0;
 	}

 	// Compute the ratio Rs/R0 and apply the environmental correction
 	float ratio = lastValueRs / valueR0 * getEnvCorrectRatio();

 	// Use the equation to compute the CL2 concentration in ppm
 	// R^2 = 0.9897
 	float ppm = 48.313 * pow(ratio, -1.179);
 	return ppm;
 }

 /**
 * Get gas concentration for O3 in ppm
 */
 float MQ131::readO3() {
 	// If no value Rs read, return 0.0
 	if(lastValueRs < 0) {
 		return 0.0;
 	}

 	// Compute the ratio Rs/R0 and apply the environmental correction
 	float ratio = lastValueRs / valueR0 * getEnvCorrectRatio();

 	// Use the equation to compute the O3 concentration in ppm
 	// R^2 = 0.9987
 	float ppm = 24.049 * pow(ratio, -1.139);
 	return ppm;
 }

 /**
  * Store R0 value (come from calibration or set by user)
  */
  void MQ131::setR0(float _valueR0) {
  	valueR0 = _valueR0;
  }

 /**
 * Get R0 value
 */
 float MQ131::getR0() {
 	return valueR0;
 }