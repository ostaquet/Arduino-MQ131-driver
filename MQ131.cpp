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

/**
 * Do a full cycle (heater, reading, stop heater)
 * The function gives back the hand only at the end
 * of the read cycle!
 */
 bool MQ131::begin() {
 	startHeater();
 	while(!isTimeToRead()) {
 		delay(1000);
 	}
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