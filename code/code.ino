#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "env.h"
// region  GPRS
// https://randomnerdtutorials.com/lilygo-t-sim7000g-esp32-lte-gprs-gps/
// SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
// if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
//   delay(10000);
//   return;
// }
// modem.sendAT("+SGPIO=0,4,1,1");
// if (modem.waitResponse(10000L) != 1) {
//   DBG(" SGPIO=0,4,1,1 false ");
// }
// endregion

// Configuración Wi-Fi taría bien que te creer el .env
// Y sacar de ahí todo lo confidencial.
const char *ssid = SSID_WIFI;
const char *password = PASSWORD_WIFI;

// Configuración de FIWARE Orion
// El endpoint
const char *orionEndpoint = "http://orion:1026/v2/entities";
// https://fiware-orion.readthedocs.io/en/master/orion-api.html#multi-tenancy
const char *fiwareServicePath = "/Almeria/AIoT/IFAPA";

// Son de ejemplo cambiar por las que haya que usar en el bucle o como se acaben leyendo los datos
const String placa = "AIOT_ESP32";
const String estacion = "EstacionIOTInvernadero";
const String nombreSensor = "temperature";
const String tipoSensor = "Number";

float getTemperature() {
  // Aquí iría el código para leer el sensor de temperatura
  return random(200, 300) / 10.0;  // Simulación: 20.0°C a 30.0°C
}

float getHumidity() {
  // Aquí iría el código para leer el sensor de humedad
  return random(400, 700) / 10.0;  // Simulación: 40.0% a 70.0%
}

int getCO2() {
  // Aquí iría el código para leer el sensor de CO2
  return random(350, 500);  // Simulación: 350ppm a 500ppm
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

void buildJsonDocument(JsonDocument &doc, const String &placa, const String &estacion, const String &nombreSensor, const String &tipoSensor, float valor) {
  doc["id"] = placa;
  doc["type"] = estacion;

  // Agregar datos de sensores
  doc[nombreSensor]["type"] = tipoSensor;
  doc[nombreSensor]["value"] = valor;
}

// Le pongo como default el path de la entidad, pero lo puedo cambiar al momento de llamar a la función
void sendToFiware(const String &jsonOutput, const String &fiwareServicePath = "/") {
  Serial.println("Enviando datos a FIWARE Orion...");
  Serial.println("JSON con formato :");
  Serial.println(jsonOutput);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(orionEndpoint);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("fiware-service", fiwareServicePath);

    int httpResponseCode = http.POST(jsonOutput);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Respuesta de FIWARE Orion:");
      Serial.println(response);
    } else {
      Serial.print("Error en la solicitud: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Error: No conectado a Wi-Fi");
  }
}

void loop() {
  StaticJsonDocument<1024> doc;
  // Datos de prueba

  float valor = getTemperature();  // Simulación de lectura de temperatura

  // Llamar al método buildJsonDocument
  buildJsonDocument(doc, placa, estacion, nombreSensor, tipoSensor, valor);

  // Voy a simular varias llamadas pare verificar que estoy montando bien el JSON
  buildJsonDocument(doc, placa, estacion, "sensor2", "tipo2", getHumidity());

  buildJsonDocument(doc, placa, estacion, "sensor3", "tipo3", getTemperature());

  buildJsonDocument(doc, placa, estacion, "sensor3", "tipo3", getCO2());

  // Serializar el JSON a un string para imprimirlo
  String jsonOutput;
  serializeJson(doc, jsonOutput);

  // Imprimir el JSON generado en el monitor serie

  // Enviar el JSON a FIWARE Orion
  sendToFiware(jsonOutput);

  delay(1000);
}