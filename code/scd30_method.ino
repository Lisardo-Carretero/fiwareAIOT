/*
By: Nathan Seidle SparkFun Electronics  
Library: http://librarymanager/All#SparkFun_SCD30  
*/

#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h"
SCD30 airSensor;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  //Serial.println("SCD30 Example");
  airSensor.begin();  //This will cause readings to occur every two seconds
}

int readCO2() {
  //if (airSensor.dataAvailable()) {
    return airSensor.getCO2();
  //} else
  //  return nan;
}

float getTempSCD30() {
  //if (airSensor.dataAvailable()) {
    return airSensor.getTemperature();
  //} else
  //  return nan;
}

float getHumSCD30() {
  //if (airSensor.dataAvailable()) {
    return airSensor.getHumidity();
  //} else
  //  return nan;
}

void loop() {
  //if (airSensor.dataAvailable()) {
  Serial.print("co2(ppm):");

  Serial.print(readCO2());

  Serial.print(" temp(C):");
  Serial.print(",");
  Serial.print(getTempSCD30(), 1);

  Serial.print(" humidity(%):");
  Serial.print(",");
  Serial.print(getHumSCD30(), 1);

  Serial.println();
  //} else
  //Serial.println("No data");

  delay(5000);
}