#include "SparkFun_SCD30_Arduino_Library.h" //Librerias
SCD30 airSensor;

void setup()
{
    Serial.begin(9600); // Velocidad de lectura del Sensor
    Serial.println("Joder que bien me lo paso");
    while (airSensor.begin() == false)
    {
        Serial.print("Sensor no detectado...");
        delay(2000);
        // while (1);
    }
    Serial.print("Lectura del sensor SCD30");
}

void loop()
{
    if (airSensor.dataAvailable())
    {
        Serial.print("co2(ppm):");
        Serial.print(airSensor.getCO2());
        Serial.print(" temp(C):");
        Serial.print(airSensor.getTemperature(), 1);
        Serial.print(" RH(%):");
        Serial.print(airSensor.getHumidity(), 1);
        Serial.println();
    }
    else
        Serial.println("Leyendo...");
    delay(5000);
}