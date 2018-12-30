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
 MQ131::MQ131(int _pinPower, int _pinSensor, MQ131Model _model, int _RL) {
 	// Setup the model
 	model = _model;

 	// Store the circuit info (pin and load resistance)
 	pinPower = _pinPower;
 	pinSensor = _pinSensor;
 	valueRL = _RL;

  // Setup default calibration value
  switch(model) {
    case LOW_CONCENTRATION :
      setR0(110470.60);
      setTimeToRead(72);
      break;
    case HIGH_CONCENTRATION :
      setR0(385.40);
      setTimeToRead(80);
      break;
  }

 	// Setup pin mode
 	pinMode(pinPower, OUTPUT);
 	pinMode(pinSensor, INPUT);

  // Switch off the heater as default status
  digitalWrite(pinPower, LOW);
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
 	secLastStart = millis()/1000;
 }

/**
 * Check if it is the right time to read the Rs value
 */
 bool MQ131::isTimeToRead() {
 	// Check if the heater has been started...
 	if(secLastStart < 0) {
 		return false;
 	}
 	// OK, check if it's the time to read based on calibration parameters
 	if(millis()/1000 >= secLastStart + getTimeToRead()) {
 		return true;
 	}
 	return false;
 } 

/**
 * Stop the heater
 */
 void MQ131::stopHeater() {
 	digitalWrite(pinPower, LOW);
 	secLastStart = -1;
 }

/**
 * Get parameter time to read
 */
 long MQ131::getTimeToRead() {
 	return secToRead;
 }

/**
 * Set parameter time to read (for calibration or to recall
 * calibration from previous run)
 */
 void MQ131::setTimeToRead(long sec) {
 	secToRead = sec;
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
 	// If default value, ignore correction ratio
 	if(humidityPercent == 60 && temperatureCelsuis == 20) {
 		return 1.0;
 	}
 	// For humidity > 75%, use the 85% curve
 	if(humidityPercent > 75) {
 		// R^2 = 0.9986
 		return -0.0141 * temperatureCelsuis + 1.5623;
 	}
 	// For humidity > 50%, use the 60% curve
 	if(humidityPercent > 50) {
 		// R^2 = 0.9976
 		return -0.0119 * temperatureCelsuis + 1.3261;
 	}

 	// Humidity < 50%, use the 30% curve
 	// R^2 = 0.996
 	return -0.0103 * temperatureCelsuis + 1.1507;
 }

 /**
 * Get gas concentration for O3 in ppm
 */
 float MQ131::getO3(MQ131Unit unit) {
 	// If no value Rs read, return 0.0
 	if(lastValueRs < 0) {
 		return 0.0;
 	}

  float ratio = 0.0;

 	switch(model) {
 		case LOW_CONCENTRATION :
 			// Use the equation to compute the O3 concentration in ppm
 			// R^2 = 0.9987
      // Compute the ratio Rs/R0 and apply the environmental correction
      ratio = lastValueRs / valueR0 * getEnvCorrectRatio();
      return convert(9.4783 * pow(ratio, 2.3348), PPB, unit);
 		case HIGH_CONCENTRATION :
 			// Use the equation to compute the O3 concentration in ppm
 			// R^2 = 0.99
      // Compute the ratio Rs/R0 and apply the environmental correction
      ratio = lastValueRs / valueR0 * getEnvCorrectRatio();
      return convert(8.1399 * pow(ratio, 2.3297), PPM, unit);
 		default :
 			return 0.0;
  }
}

 /**
  * Convert gas unit of gas concentration
  */
 float MQ131::convert(float input, MQ131Unit unitIn, MQ131Unit unitOut) {
  if(unitIn == unitOut) {
    return input;
  }

  float concentration = 0;

  switch(unitOut) {
    case PPM :
      // We assume that the unit IN is PPB as the sensor provide only in PPB and PPM
      // depending on the type of sensor (METAL or BLACK_BAKELITE)
      // So, convert PPB to PPM
      return input / 1000.0;
    case PPB :
      // We assume that the unit IN is PPM as the sensor provide only in PPB and PPM
      // depending on the type of sensor (METAL or BLACK_BAKELITE)
      // So, convert PPM to PPB
      return input * 1000.0;
    case MG_M3 :
      if(unitIn == PPM) {
        concentration = input;
      } else {
        concentration = input / 1000.0;
      }
      return concentration * 48.0 / 22.71108;
    case UG_M3 :
      if(unitIn == PPB) {
        concentration = input;
      } else {
        concentration = input * 1000.0;
      }
      return concentration * 48.0 / 22.71108;
    default :
      return input;
  }
}

 /**
  * Calibrate the basic values (R0 and time to read)
  */
void MQ131::calibrate() {
  // Take care of the last Rs value read on the sensor
  // (forget the decimals)
  float lastRsValue = 0;
  // Count how many time we keep the same Rs value in a row
  int countReadInRow = 0;
  // Count how long we have to wait to have consistent value
  int count = 0;

  // Start heater
  startHeater();

  int timeToReadConsistency = -1;
  switch(model) {
    case LOW_CONCENTRATION :
      timeToReadConsistency = 15;
      break;
    case HIGH_CONCENTRATION :
      timeToReadConsistency = 20;
      break;
  }

  while(countReadInRow <= timeToReadConsistency) {
    float value = readRs();
    if((int)lastRsValue != (int)value) {
      lastRsValue = value;
      countReadInRow = 0;
    } else {
      countReadInRow++;
    }
    count++;
    delay(1000);
  }

  // Stop heater
  stopHeater();

  // We have our R0 and our time to read
  setR0(lastRsValue);
  setTimeToRead(count);
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
