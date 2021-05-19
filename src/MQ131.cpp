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

#include "MQ131.h"

/**
 * Constructor, nothing special to do
 */
MQ131Class::MQ131Class(uint32_t _RL) {
  valueRL = _RL;
}

/**
 * Destructor, nothing special to do
 */
MQ131Class::~MQ131Class() {
}

/**
 * Init core variables
 */
 void MQ131Class::begin(uint8_t _pinPower, uint8_t _pinSensor, MQ131Model _model, uint32_t _RL, Stream* _debugStream) { 
  // Define if debug is requested
  enableDebug = _debugStream != NULL;
  debugStream = _debugStream;
  
 	// Setup the model
 	model = _model;

 	// Store the circuit info (pin and load resistance)
 	pinPower = _pinPower;
 	pinSensor = _pinSensor;
 	valueRL = _RL;

  // Setup default calibration value
  switch(model) {
    case LOW_CONCENTRATION :
      setR0(MQ131_DEFAULT_LO_CONCENTRATION_R0);
      setTimeToRead(MQ131_DEFAULT_LO_CONCENTRATION_TIME2READ);
      break;
    case HIGH_CONCENTRATION :
      setR0(MQ131_DEFAULT_HI_CONCENTRATION_R0);
      setTimeToRead(MQ131_DEFAULT_HI_CONCENTRATION_TIME2READ);
      break; 
    case SN_O2_LOW_CONCENTRATION:
      // Not tested by @ostaquet (I don't have this type of sensor)
      setR0(MQ131_DEFAULT_LO_CONCENTRATION_R0);
      setTimeToRead(MQ131_DEFAULT_LO_CONCENTRATION_TIME2READ);
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
 void MQ131Class::sample() {
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
 void MQ131Class::startHeater() {
 	digitalWrite(pinPower, HIGH);
 	secLastStart = millis()/1000;
 }

/**
 * Check if it is the right time to read the Rs value
 */
 bool MQ131Class::isTimeToRead() {
 	// Check if the heater has been started...
 	if(secLastStart < 0) {
 		return false;
 	}
 	// OK, check if it's the time to read based on calibration parameters
 	if(millis() / 1000 >= secLastStart + getTimeToRead()) {
 		return true;
 	}
 	return false;
 } 

/**
 * Stop the heater
 */
 void MQ131Class::stopHeater() {
 	digitalWrite(pinPower, LOW);
 	secLastStart = -1;
 }

/**
 * Get parameter time to read
 */
 long MQ131Class::getTimeToRead() {
 	return secToRead;
 }

/**
 * Set parameter time to read (for calibration or to recall
 * calibration from previous run)
 */
 void MQ131Class::setTimeToRead(uint32_t sec) {
 	secToRead = sec;
 }

/**
 * Read Rs value
 */
 float MQ131Class::readRs() {
 	// Read the value
 	uint16_t valueSensor = analogRead(pinSensor);
 	// Compute the voltage on load resistance (for 5V Arduino)
  float vRL = ((float)valueSensor) / 1024.0 * 5.0;
 	// Compute the resistance of the sensor (for 5V Arduino)
  if(!vRL) return 0.0f; //division by zero prevention
 	float rS = (5.0 / vRL - 1.0) * valueRL;
 	return rS;
 }

/**
 * Set environmental values
 */
 void MQ131Class::setEnv(int8_t tempCels, uint8_t humPc) {
 	temperatureCelsuis = tempCels;
 	humidityPercent = humPc;
 }

/**
 * Get correction to apply on Rs depending on environmental
 * conditions
 */
 float MQ131Class::getEnvCorrectRatio() {
 	// Select the right equation based on humidity
 	// If default value, ignore correction ratio
 	if(humidityPercent == 60 && temperatureCelsuis == 20) {
 		return 1.0;
 	}
 	// For humidity > 75%, use the 85% curve
 	if(humidityPercent > 75) {
    // R^2 = 0.996
   	return -0.0103 * temperatureCelsuis + 1.1507;
 	}
 	// For humidity > 50%, use the 60% curve
 	if(humidityPercent > 50) {
 		// R^2 = 0.9976
 		return -0.0119 * temperatureCelsuis + 1.3261;
 	}

 	// Humidity < 50%, use the 30% curve
  // R^2 = 0.9986
 	return -0.0141 * temperatureCelsuis + 1.5623;
 }

 /**
 * Get gas concentration for O3 in ppm
 */
 float MQ131Class::getO3(MQ131Unit unit) {
 	// If no value Rs read, return 0.0
 	if(lastValueRs < 0) {
 		return 0.0;
 	}

  float ratio = 0.0;

 	switch(model) {
 		case LOW_CONCENTRATION :
 			// Use the equation to compute the O3 concentration in ppm
      // Compute the ratio Rs/R0 and apply the environmental correction
      ratio = lastValueRs / valueR0 * getEnvCorrectRatio();
      // R^2 = 0.9906
      // Use this if you are monitoring low concentration of O3 (air quality project)
      return convert(9.4783 * pow(ratio, 2.3348), PPB, unit);
      
      // R^2 = 0.9986 but nearly impossible to have 0ppb
      // Use this if you are constantly monitoring high concentration of O3
      // return convert((10.66435681 * pow(ratio, 2.25889394) - 10.66435681), PPB, unit);

 		case HIGH_CONCENTRATION :
 			// Use the equation to compute the O3 concentration in ppm
 			
      // Compute the ratio Rs/R0 and apply the environmental correction
      ratio = lastValueRs / valueR0 * getEnvCorrectRatio();
      // R^2 = 0.9900
      // Use this if you are monitoring low concentration of O3 (air quality project)
      return convert(8.1399 * pow(ratio, 2.3297), PPM, unit);
      
      // R^2 = 0.9985 but nearly impossible to have 0ppm
      // Use this if you are constantly monitoring high concentration of O3
      // return convert((8.37768358 * pow(ratio, 2.30375446) - 8.37768358), PPM, unit);

    case SN_O2_LOW_CONCENTRATION:
      // NOT TESTED BY @ostaquet (I don't have this type of sensor)
      ratio = 12.15* lastValueRs / valueR0 * getEnvCorrectRatio();
      // r^2 = 0.9956
      return convert(26.941 * pow(ratio,-1.16),PPB,unit);
      break;
      
 		default :
 			return 0.0;
  }
}

 /**
  * Convert gas unit of gas concentration
  */
 float MQ131Class::convert(float input, MQ131Unit unitIn, MQ131Unit unitOut) {
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
void MQ131Class::calibrate() {
  // Take care of the last Rs value read on the sensor
  // (forget the decimals)
  float lastRsValue = 0;
  float lastLastRsValue = 0;
  // Count how many time we keep the same Rs value in a row
  uint8_t countReadInRow = 0;
  // Count how long we have to wait to have consistent value
  uint8_t count = 0;

  // Get some info
  if(enableDebug) {
    debugStream->println(F("MQ131 : Starting calibration..."));
    debugStream->println(F("MQ131 : Enable heater"));
    debugStream->print(F("MQ131 : Stable cycles required : "));
    debugStream->print(MQ131_DEFAULT_STABLE_CYCLE);
    debugStream->println(F(" (compilation parameter MQ131_DEFAULT_STABLE_CYCLE)"));
  }

  // Start heater
  startHeater();

  uint8_t timeToReadConsistency = MQ131_DEFAULT_STABLE_CYCLE;

  while(countReadInRow <= timeToReadConsistency) {
    float value = readRs();

    if(enableDebug) {
      debugStream->print(F("MQ131 : Rs read = "));
      debugStream->print((uint32_t)value);
      debugStream->println(F(" Ohms"));
    }
    
    if((uint32_t)lastRsValue != (uint32_t)value && (uint32_t)lastLastRsValue != (uint32_t)value) {
      lastLastRsValue = lastRsValue;
      lastRsValue = value;
      countReadInRow = 0;
    } else {
      countReadInRow++;
    }
    count++;
    delay(1000);
  }

  if(enableDebug) {
    debugStream->print(F("MQ131 : Stabilisation after "));
    debugStream->print(count);
    debugStream->println(F(" seconds"));
    debugStream->println(F("MQ131 : Stop heater and store calibration parameters"));
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
  void MQ131Class::setR0(float _valueR0) {
  	valueR0 = _valueR0;
  }

 /**
 * Get R0 value
 */
 float MQ131Class::getR0() {
 	return valueR0;
 }

MQ131Class MQ131(MQ131_DEFAULT_RL);
