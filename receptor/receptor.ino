#include <ArduinoJson.h>

void setup()
{
    Serial.begin(9600);

    Serial.println("Arduino Mega listo para recibir datos...");
}

void loop()
{
    // Verificar si hay datos disponibles desde el ESP32
    if (Serial.available())
    {
        String jsonData = Serial.readStringUntil('\n'); // Leer los datos enviados por el ESP32

        // Parsear el JSON recibido
        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, jsonData);

        // Extraer los valores del JSON
        float temperature = doc["temperature"];
        float humidity = doc["humidity"];

        // Mostrar los datos en el monitor serie
        Serial.print("Temperatura: ");
        Serial.println(temperature);
        Serial.print("Humedad: ");
        Serial.println(humidity);
    }
}