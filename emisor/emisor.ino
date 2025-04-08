#include <ArduinoJson.h>

// Configuración del puerto serial para comunicación con el Arduino Mega
#define SERIAL_BAUD_RATE 9600

void setup()
{
    Serial.begin(9600);

    Serial.println("ESP32 listo para enviar datos...");
}

void loop()
{
    // Crear datos simulados
    StaticJsonDocument<128> doc;
    doc["temperature"] = random(200, 300) / 10.0; // 20.0°C a 30.0°C
    doc["humidity"] = random(400, 700) / 10.0;    // 40.0% a 70.0%

    // Serializar los datos a un string
    String jsonData;
    serializeJson(doc, jsonData);

    // Enviar los datos al Arduino Mega
    Serial.println("Datos enviados: " + jsonData);

    delay(2000); // Enviar cada 2 segundos
}