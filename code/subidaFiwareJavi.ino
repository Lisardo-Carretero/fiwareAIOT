#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "env.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define SSID_WIFI "Smartphone-SEU"
#define PASSWORD_WIFI "SistemasEmpotrados"

//DHT sensors
#define DHTPIN1 4       // Digital pin connected to the DHT sensor
#define DHTTYPE1 DHT11  // DHT 1
DHT_Unified dht1(DHTPIN1, DHTTYPE1);

#define DHTPIN2 15      // Digital pin connected to the DHT sensor
#define DHTTYPE2 DHT22  // DHT 1
DHT_Unified dht2(DHTPIN2, DHTTYPE2);

// Configuration
const char *ssid = SSID_WIFI;
const char *password = PASSWORD_WIFI;
const char *orionEndpoint = "http://192.168.219.224:1026/v2/entities";
// const char *fiwareService = ""; // Only alphanumeric and underscore
const char *fiwareServicePath = "/AlmeriaAIoT"; // Path can have slashes

// Entity configuration
const String entityId = "EstacionAIOT-SmartHome";
const String entityType = "EstacionAIOT";

//Radiation sensor
const int sensorPin6 = 32;
int sensorValue1;  // variable que almacena el valor raw (0 a 1023)
float value1;      // variable que almacena el voltaje (0.0 a 5.0)


// Simulated sensor readings (replace with actual sensor code)
float getTemperature(DHT_Unified dht_sensor) {
  sensors_event_t event;
  dht_sensor.temperature().getEvent(&event);
  return event.temperature;
}

float getHumidity(DHT_Unified dht_sensor) {
  sensors_event_t event;
  dht_sensor.humidity().getEvent(&event);
  return event.relative_humidity;
}


float getSoilMoisture() {
  return random(50, 250) / 10.0;
}

float getSoilDielectric() {
  return random(20, 50) / 10.0;
}

float getSoilConductivity() {
  return random(30, 80) / 10.0;
}

float getSoilTemperature() {
  return random(180, 260) / 10.0;
}

float getRadiation() {
  sensorValue1 = analogRead(sensorPin6);           // realizar la lectura
  Serial.println(sensorValue1);
  value1 = fmap(sensorValue1, 0, 1023, 0.0, 5.0);  // cambiar escala a 0.0 - 5.0
  Serial.println(value1);
  float tension_PPFD = value1 * 100;
  float PPFD = 5 * tension_PPFD;
  Serial.println(PPFD);
  return PPFD;
}

// scale change neded to calculate radiation
float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Add a sensor reading to the JSON document only if it's valid
void addSensorReading(JsonDocument &doc, const String &sensorName, float value) {
  // Check if the value is valid (not NaN)
  if (!isnan(value)) {
    doc[sensorName]["type"] = "float";
    doc[sensorName]["value"] = value;
  }
}

// Initialize the JSON document with entity and type
void buildJsonDocument(JsonDocument &doc) {
  doc["id"] = entityId;
  doc["type"] = entityType;
}

void sendToFiware(const String &jsonOutput) {
  Serial.println("Enviando datos a FIWARE Orion...");
  Serial.println("JSON con formato:");
  Serial.println(jsonOutput);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setTimeout(10000); // 10 second timeout
    
    // First try to create the entity
    http.begin(orionEndpoint);
    http.addHeader("Content-Type", "application/json");
    // http.addHeader("fiware-service", fiwareService);
    http.addHeader("fiware-servicepath", fiwareServicePath);

    int httpResponseCode = http.POST(jsonOutput);
    Serial.print("POST response code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 422) {
      // Entity already exists, update it instead
      Serial.println("Entity already exists, updating...");
      http.end();
      
      // Format endpoint for update operation
      String updateEndpoint = String(orionEndpoint) + "/" + entityId + "/attrs";
      Serial.print("Update endpoint: ");
      Serial.println(updateEndpoint);
      
      http.begin(updateEndpoint);
      http.addHeader("Content-Type", "application/json");
      // http.addHeader("fiware-service", fiwareService);
      http.addHeader("fiware-servicepath", fiwareServicePath);
      
      // Remove the id and type from the JSON for update
      StaticJsonDocument<1024> updateDoc;
      DeserializationError error = deserializeJson(updateDoc, jsonOutput);
      
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        http.end();
        return;
      }
      
      updateDoc.remove("id");
      updateDoc.remove("type");
      
      String updateJson;
      serializeJson(updateDoc, updateJson);
      
      Serial.println("Update JSON:");
      Serial.println(updateJson);
      
      // Try PUT instead of PATCH (more widely supported)
      httpResponseCode = http.PUT(updateJson);
      Serial.print("PUT response code: ");
      Serial.println(httpResponseCode);
      
      // 204 is a success code for PUT with no content returned
      if (httpResponseCode == 204) {
        Serial.println("Update successful (No Content)");
      }
    }

    // Process response depending on status code
    if (httpResponseCode > 0) {
      // Only try to get response string if it's not a 204 (No Content) response
      if (httpResponseCode != 204) {
        String response = http.getString();
        Serial.println("Respuesta de FIWARE Orion:");
        Serial.println(response);
      }
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);
      Serial.println("Request completed successfully");
    } else {
      Serial.print("Error en la solicitud: ");
      Serial.println(httpResponseCode);
    }

    http.end();
    Serial.println("HTTP client closed");
  } else {
    Serial.println("Error: No conectado a Wi-Fi");
  }
  
  Serial.println("Function sendToFiware completed");
}

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConectado a Wi-Fi");

  // Initialize dht sensors.
  dht1.begin();
  dht2.begin();
}

void loop() {
  StaticJsonDocument<1024> doc;
    
  // Initialize the document with entity and type
  buildJsonDocument(doc);
  
  // Get sensor readings (these could return NaN or valid values)
  float dht11Hum = getHumidity(dht1); 
  float dht11Temp = getTemperature(dht1); 
  float dht22Temp = getTemperature(dht2);
  float dht22Hum = getHumidity(dht2);
  float soilMoisture = getSoilMoisture();
  float soilDielectric = getSoilDielectric();
  float soilConductivity = getSoilConductivity();
  float soilTemp = getSoilTemperature();
  float radiation = getRadiation();
  
  // Add each sensor reading to the document (only if valid)
  addSensorReading(doc, "sDHT11_Temperatura", dht11Temp);
  addSensorReading(doc, "sDHT11_Humedad", dht11Hum);
  addSensorReading(doc, "sDHT22_Temperatura", dht22Temp);
  addSensorReading(doc, "sDHT22_Humedad", dht22Hum);
  addSensorReading(doc, "sSuelo_Agua", soilMoisture);
  addSensorReading(doc, "sSuelo_Dielectricidad", soilDielectric);
  addSensorReading(doc, "sSuelo_Conductividad", soilConductivity);
  addSensorReading(doc, "sSuelo_Temperatura", soilTemp);
  addSensorReading(doc, "sRadiacion", radiation);
  
  // Serialize the JSON to a string
  String jsonOutput;
  serializeJson(doc, jsonOutput);
  
  // Send the JSON to FIWARE Orion
  sendToFiware(jsonOutput);
  //Serial.println(jsonOutput);

  
  delay(4000); // Send data every 10 seconds
}