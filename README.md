# Arduino-MQ131-driver
Arduino library for ozone gas sensor MQ131

 This is a comprehensive Arduino library to obtain ozone (O3) concentration in the air with the Winsen MQ131 sensor. The library supports both versions of the sensor (low concentration and high concentration), the calibration, the control of the heater, the environmental adjustments (temperature and humidity) and the output of values in ppm (parts per million), ppb (parts per billion), mg/m3 and µg/m3.

## To know before starting...
 * The MQ131 is a [semiconductor gas sensor](https://en.wikipedia.org/wiki/Gas_detector#Semiconductor) composed by a heater circuit and a sensor circuit.
 * Heater consumes at least 150 mA. So, __don't connect it directly on a pin of the Arduino__.
 * Sensor MQ131 requires minimum 48h preheat time before giving consistent results (also called "burn-in" time)
 * There are two different MQ131; a black bakelite sensor for low concentration of ozone and a metal sensor for high concentration of ozone.
 * This driver is made to control the "naked" [Winsen](https://www.winsen-sensor.com) MQ131. The driver is able to pilot the [low concentration version](https://github.com/ostaquet/Arduino-MQ131-driver/blob/master/datasheet/MQ131-low-concentration.pdf) and the [high concentration version](https://github.com/ostaquet/Arduino-MQ131-driver/blob/master/datasheet/MQ131-high-concentration.pdf).
 * To measure the air quality (e.g. pollution), it's better to use the low concentration MQ131 because the high concentration is not accurate enough for low concentration.
 
## How to install the library?
 1. Click on *Clone or download* on GitHub
 2. Choose *Download ZIP*
 3. Open Arduino IDE
 4. In the menu *Sketch* -> *Include Library* -> *Add .ZIP Library*
 5. Select the downloaded file or folder (if automatically uncompress)
 6. Restart Arduino IDE
 7. Enjoy!
 
## Circuit
 * Heater is controlled by NPN transistor via the control pin (on schema pin 2, yellow connector)
 * Result of the sensor is read through analog with RL of 10kΩ (on schema pin A0, green connector)
 
![Breadboard schematics](img/MQ131_bb.png)

![Schematics](img/MQ131_schem.png)

## Basic program to use your MQ131
```
#include "MQ131.h"

// Init the sensor
// - Heater control on pin 2
// - Sensor analog read on pin A0
// - Model LOW_CONCENTRATION
// - Load resistance RL of 10KOhms (10000 Ohms)
MQ131 sensor(2,A0, LOW_CONCENTRATION, 10000);

void setup() {
  Serial.begin(115200);

  Serial.println("Calibration in progress...");

  sensor.calibrate();

  Serial.println("Calibration done!");
  Serial.print("R0 = ");
  Serial.print(sensor.getR0());
  Serial.println(" Ohms");
  Serial.print("Time to heat = ");
  Serial.print(sensor.getTimeToRead());
  Serial.println(" s");
}

void loop() {
  sensor.begin();

  Serial.print("Concentration O3 : ");
  Serial.print(sensor.getO3(PPM));
  Serial.println(" ppm");
  Serial.print("Concentration O3 : ");
  Serial.print(sensor.getO3(PPB));
  Serial.println(" ppb");
  Serial.print("Concentration O3 : ");
  Serial.print(sensor.getO3(MG_M3));
  Serial.println(" mg/m3");
  Serial.print("Concentration O3 : ");
  Serial.print(sensor.getO3(UG_M3));
  Serial.println(" ug/m3");

  delay(60000);
}
```

The result gives us:
```
Calibration in progress...
Calibration done!
R0 = 110470.60 Ohms
Time to heat = 72 s
Sampling...
Concentration O3 : 0.01 ppm
Concentration O3 : 11.90 ppb
Concentration O3 : 0.03 mg/m3
Concentration O3 : 25.14 ug/m3
```

## Usage
The driver has to be initialized with 4 parameters:
 * Pin to control the heater power (example: 2)
 * Pin to measure the analog output (example: A0)
 * Model of sensor LOW_CONCENTRATION or HIGH_CONCENTRATION (example: LOW_CONCENTRATION)
 * Value of load resistance in Ohms (example: 10000 Ohms)
```
MQ131 sensor(2,A0, LOW_CONCENTRATION, 10000);
```

Before using the driver, it's better to calibrate it. You can do that through the function calibrate(). The best is to calibrate the sensor at 20°C and 65% of humidity in clean fresh air. The calibration adjusts 2 parameters:
 * The value of the base resistance (R0)
 * The time required to heat the sensor and get consistent readings (Time to read)
```
sensor.calibrate();
```

Those calibration values are used for the usage of the sensor as long as the Arduino is not restarted. Nevertheless, you can get the values for your sensor through the getters:
```
sensor.getR0();
sensor.getTimeToRead();
```

And set up the values in the initialization of your program through the setters:
```
sensor.setR0(value);
sensor.setTimeToRead(value);
```

In order to get the values from the sensor, you just start the process with the begin() function. Please notice that the function locks the flow. If you want to do additional processing during the heating/reading process, you should extend the class. The methods are protected and the driver can be extended easily.
```
sensor.begin();
```

The reading of the values is done through the getO3() function. Based on the parameter, you can ask to receive the result in ppm (PPM), ppb (PPB), mg/m3 (MG_M3) or µg/m3 (UG_M3).
```
sensor.getO3(PPM);
sensor.getO3(PPB);
sensor.getO3(MG_M3);
sensor.getO3(UG_M3);
```

The sensor is sensible to environmental variation (temperature and humidity). If you want to have correct values, you should set the temperature and the humidity before the call to getO3() function with the function setEnv(). Temperature are in °C and humidity in %. The values should come from another sensor like the DHT22.
```
sensor.setEnv(23, 70);
```


## Links
 * [Calculation of sensitivity curves](https://github.com/ostaquet/Arduino-MQ131-driver/blob/master/datasheet/Sensitivity_curves.xlsx)
 * [Datasheet MQ131 low concentration (black bakelite version)](https://github.com/ostaquet/Arduino-MQ131-driver/blob/master/datasheet/MQ131-low-concentration.pdf)
 * [Datasheet MQ131 high concentration (metal version)](https://github.com/ostaquet/Arduino-MQ131-driver/blob/master/datasheet/MQ131-high-concentration.pdf)
