#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Configuración Wi-Fi
const char *ssid = "TU_SSID";
const char *password = "TU_PASSWORD";

// Configuración de FIWARE Orion
const char *orionEndpoint = "http://orion:1026/v2/entities";
const char *fiwareService = "openiot";
const char *fiwareServicePath = "/";

// Datos de la entidad
String entityId = "AIOT_ESP32";
String entityType = "EstacionIOTInvernadero";

float getTemperature()
{
    // Aquí iría el código para leer el sensor de temperatura
    return random(200, 300) / 10.0; // Simulación: 20.0°C a 30.0°C
}

float getHumidity()
{
    // Aquí iría el código para leer el sensor de humedad
    return random(400, 700) / 10.0; // Simulación: 40.0% a 70.0%
}

int getCO2()
{
    // Aquí iría el código para leer el sensor de CO2
    return random(350, 500); // Simulación: 350ppm a 500ppm
}

void setup()
{
    Serial.begin(96000);
    WiFi.begin(ssid, password);

    Serial.print("Conectando a Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConectado a Wi-Fi");
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;

        // Crear el JSON dinámicamente
        StaticJsonDocument<1024> doc;
        doc["id"] = entityId;
        doc["type"] = entityType;

        // Agregar datos de sensores
        doc["temperature"]["type"] = "Number";
        doc["temperature"]["value"] = getTemperature();

        doc["relativeHumidity"]["type"] = "Number";
        doc["relativeHumidity"]["value"] = getHumidity();

        doc["CO2"]["type"] = "Number";
        doc["CO2"]["value"] = getCO2();

        // Agregar ubicación
        doc["location"]["type"] = "geo:json";
        JsonObject locationValue = doc["location"]["value"].to<JsonObject>();
        locationValue["type"] = "Point";
        JsonArray coordinates = locationValue.createNestedArray("coordinates");
        coordinates.add(-99.133167); // Longitud
        coordinates.add(19.434072);  // Latitud

        // Agregar fecha y hora
        doc["dateObserved"]["type"] = "DateTime";
        doc["dateObserved"]["value"] = "2025-04-03T12:00:00Z"; // Cambiar por la hora actual si es necesario

        // Serializar el JSON a un string
        String payload;
        serializeJson(doc, payload);

        // Configurar la solicitud HTTP
        http.begin(orionEndpoint);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Fiware-Service", fiwareService);
        http.addHeader("Fiware-ServicePath", fiwareServicePath);

        // Enviar la solicitud POST
        int httpResponseCode = http.POST(payload);

        // Manejar la respuesta
        if (httpResponseCode > 0)
        {
            String response = http.getString();
            Serial.println("Respuesta del servidor:");
            Serial.println(response);
        }
        else
        {
            Serial.print("Error en la solicitud: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    }
    else
    {
        Serial.println("Wi-Fi no conectado");
    }

    delay(60000); // Esperar 1 minuto antes de enviar nuevamente
}