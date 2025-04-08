#include <ArduinoJson.h>

void setup() {
  Serial.begin(9600);  // Comunicación con el Arduino Mega (RX/TX)
  Serial.println("ESP32 listo para recibir datos...");
}

void loop() {
  if (Serial.available()) {
    String jsonData = Serial.readStringUntil('\n');  // Leer datos del Arduino Mega

    // Parsear el JSON recibido
    StaticJsonDocument<256> doc;  // Aumentar el tamaño si el JSON es más grande
    deserializeJson(doc, jsonData);

    // Acceder a cada valor del JSON y mostrarlo en el monitor serie
    float dielMean = doc["Dielectrico"];
    float soilMoistMean = doc["Volumen"];
    float bulkECMean = doc["Conductividad"];
    float degCMean = doc["Temperature"];
    float solutionECMean = doc["SolutionEC"];
    float porewaterECMean = doc["PorewaterEC"];

    Serial.println("Datos recibidos:");
    Serial.print("Temperatura: ");
    Serial.println(degCMean);
    Serial.print("Componente Dielectrica: ");
    Serial.println(dielMean);
    Serial.print("Volumen: ");
    Serial.println(soilMoistMean);
    Serial.print("Conductividad: ");
    Serial.println(bulkECMean);
    Serial.print("Solution EC: ");
    Serial.println(solutionECMean);
    Serial.print("Porewater EC: ");
    Serial.println(porewaterECMean);
  }
}