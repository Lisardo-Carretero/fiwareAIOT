#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "env.h"

#define SSID_WIFI "Smartphone-SEU"
#define PASSWORD_WIFI "SistemasEmpotrados"

// Configuration
const char *ssid = SSID_WIFI;
const char *password = PASSWORD_WIFI;

const char *orionEndpoint = "http://192.168.219.224:1026/v2/entities";
// const char *fiwareService = ""; // Only alphanumeric and underscore
const char *fiwareServicePath = "/AlmeriaAIoT"; // Path can have slashes

// Entity configuration
const String entityId = "EstacionAIOT-SmartHome";
const String entityType = "EstacionAIOT";

// Simulated sensor readings (replace with actual sensor code)
float getDHT11Temperature() {
  return random(200, 300) / 10.0;
}

float getDHT11Humidity() {
  return random(400, 700) / 10.0;
}

float getDHT22Temperature() {
  return random(200, 300) / 10.0;
}

float getDHT22Humidity() {
  return random(400, 700) / 10.0;
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
  return random(150, 300) / 10.0;
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

void loop() {
  StaticJsonDocument<1024> doc;
    
  // Initialize the document with entity and type
  buildJsonDocument(doc);
  
  // Get sensor readings (these could return NaN or valid values)
  float dht11Hum = getDHT11Humidity(); 
  float dht11Temp = getDHT11Temperature(); 
  float dht22Temp = getDHT22Temperature();
  float dht22Hum = getDHT22Humidity();
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
  
  delay(4000); // Send data every 10 seconds
}