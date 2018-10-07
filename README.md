# Arduino-MQ131-driver
Arduino driver for gas sensor MQ131 (O3, NOx and CL2)

## To know before started...
 * The MQ131 is a gas sensor composed by a heater circuit and a sensor circuit.
 * Heater consumes 150mA; don't connect it directly on a pin of the Arduino.
 * Sensor MQ131 requires minimum 48h pre-heat time before giving consistent results.
 * This driver is made to control a "naked" MQ131.
 
## Circuit
 * Heater is controlled by NPN transistor via the control pin (on schema PIN 2, yellow connector)
 * Result of sensor is read through analog (on schema pin A0, green connector)
 
![Breadboard schematics](img/MQ131_bb.png)

![Schematics](img/MQ131_schem.png)

## Links
 * [Datasheet MQ131](https://github.com/ostaquet/Arduino-MQ131-driver/blob/master/datasheet/MQ131.pdf)
