#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h"

#define SSID_WIFI "Smartphone-SEU"
#define PASSWORD_WIFI "SistemasEmpotrados"

// DHT sensors
#define DHTPIN1 4       // Digital pin connected to the DHT sensor
#define DHTTYPE1 DHT11  // DHT 1
DHT_Unified dht1(DHTPIN1, DHTTYPE1);

#define DHTPIN2 15      // Digital pin connected to the DHT sensor
#define DHTTYPE2 DHT22  // DHT 1
DHT_Unified dht2(DHTPIN2, DHTTYPE2);

// Configuration
const char *ssid = SSID_WIFI;
const char *password = PASSWORD_WIFI;
const char *orionEndpoint = "http://192.168.158.224:1026/v2/entities";
const char *fiwareServicePath = "/AlmeriaAIoT";  // Path can have slashes

// Entity configuration
const String entityId = "EstacionAIOT-SmartHome";
const String entityType = "EstacionAIOT";

// Radiation sensor
const int sensorPin6 = 32;
int sensorValue1;  // variable que almacena el valor raw (0 a 1023)
float value1;      // variable que almacena el voltaje (0.0 a 5.0)

// Air sensor
SCD30 airSensor;

// Task handles for dual-core operation
TaskHandle_t soilTaskHandle;

// Mutex for protecting shared data
portMUX_TYPE soilDataMutex = portMUX_INITIALIZER_UNLOCKED;

// Shared soil data structure
struct SoilData {
  float dielectric;
  float moisture;
  float conductivity;
  float temperature;
  float solutionEC;
  float porewaterEC;
  bool dataReady;
} soilData;

// Sensor readings functions
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

float getRadiation() {
  sensorValue1 = analogRead(sensorPin6);
  value1 = fmap(sensorValue1, 0, 1023, 0.0, 5.0);
  float tension_PPFD = value1 * 100;
  float PPFD = 5 * tension_PPFD;
  return PPFD;
}

float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int readCO2() {
  return airSensor.getCO2();
}

float getTempSCD30() {
  return airSensor.getTemperature();
}

float getHumSCD30() {
  return airSensor.getHumidity();
}

// Function that runs on core 0 to handle soil data collection
void soilTask(void *parameter) {
  for (;;) {
    if (Serial.available()) {
      String jsonData = Serial.readStringUntil('\n');  // Leer datos del Arduino Mega

      // Parse JSON data
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, jsonData);

      if (!error) {
        // Lock the mutex before updating shared data
        portENTER_CRITICAL(&soilDataMutex);

        // Update the soil data structure
        soilData.dielectric = doc["Dielectrico"];
        soilData.moisture = doc["Volumen"];
        soilData.conductivity = doc["Conductividad"];
        soilData.temperature = doc["Temperature"];
        soilData.solutionEC = doc["SolutionEC"];
        soilData.porewaterEC = doc["PorewaterEC"];
        soilData.dataReady = true;

        // Release the mutex
        portEXIT_CRITICAL(&soilDataMutex);
      } else {
        Serial.println("Error en el core");
        Serial.println(xPortGetCoreID());
        Serial.print("JSON parsing error: ");
        Serial.println(error.c_str());
      }
    }

    // Give other tasks some time to run
    delay(100);
  }
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
    http.setTimeout(10000);  // 10 second timeout

    // First try to create the entity
    http.begin(orionEndpoint);
    http.addHeader("Content-Type", "application/json");
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

  // Initialize soil data
  soilData.dataReady = false;

  WiFi.begin(ssid, password);

  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConectado a Wi-Fi");

  // Initialize sensors
  dht1.begin();
  dht2.begin();
  airSensor.begin();

  // Create the soil task on Core 0
  xTaskCreatePinnedToCore(
    soilTask,         // Function to implement the task
    "SoilTask",       // Name of the task
    4096,             // Stack size in words
    NULL,             // Task input parameter
    1,                // Priority of the task
    &soilTaskHandle,  // Task handle
    0                 // Core where the task should run
  );
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

  // Get soil data safely using the mutex
  float soilDielectric = 0;
  float soilMoisture = 0;
  float soilConductivity = 0;
  float sSueloTemperatura = 0;
  float sSueloSolution = 0;
  float sSueloPorewater = 0;
  bool hasSoilData = false;

  // Enter critical section
  portENTER_CRITICAL(&soilDataMutex);

  // Copy data from the shared structure
  if (soilData.dataReady) {
    soilDielectric = soilData.dielectric;
    soilMoisture = soilData.moisture;
    soilConductivity = soilData.conductivity;
    sSueloTemperatura = soilData.temperature;
    sSueloSolution = soilData.solutionEC;
    sSueloPorewater = soilData.porewaterEC;
    hasSoilData = true;
  }

  // Exit critical section
  portEXIT_CRITICAL(&soilDataMutex);

  // Get other sensor readings
  float radiation = getRadiation();

  // Read air sensor
  delay(1000);  // Give some time for SCD30 measurements
  int CO2 = readCO2();
  float scd30Temp = getTempSCD30();
  float scd30Hum = getHumSCD30();

  // Add each sensor reading to the document (only if valid)
  addSensorReading(doc, "sDHT11_Temperatura", dht11Temp);
  addSensorReading(doc, "sDHT11_Humedad", dht11Hum);
  addSensorReading(doc, "sDHT22_Temperatura", dht22Temp);
  addSensorReading(doc, "sDHT22_Humedad", dht22Hum);

  // Only add soil data if we have valid readings
  if (hasSoilData) {
    addSensorReading(doc, "sSuelo_Agua", soilMoisture);
    addSensorReading(doc, "sSuelo_Dielectricidad", soilDielectric);
    addSensorReading(doc, "sSuelo_Conductividad", soilConductivity);
    addSensorReading(doc, "sSuelo_Temperatura", sSueloTemperatura);
    addSensorReading(doc, "sSuelo_SolutionEC", sSueloSolution);
    addSensorReading(doc, "sSuelo_Porewater", sSueloPorewater);
  }

  addSensorReading(doc, "sRadiacion", radiation);
  addSensorReading(doc, "sAire_CO2", CO2);
  addSensorReading(doc, "sAire_Temperatura", scd30Temp);
  addSensorReading(doc, "sAire_Humedad", scd30Hum);

  // Serialize the JSON to a string
  String jsonOutput;
  serializeJson(doc, jsonOutput);

  // Send the JSON to FIWARE Orion
  sendToFiware(jsonOutput);

  delay(4000);  // Send data every 4 seconds
}