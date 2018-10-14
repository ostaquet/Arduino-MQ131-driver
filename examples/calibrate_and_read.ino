#include "MQ131.h"

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
  Serial.println("Sampling...");
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