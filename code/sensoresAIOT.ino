#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN1 4       // Digital pin connected to the DHT sensor
#define DHTTYPE1 DHT11  // DHT 1
DHT_Unified dht1(DHTPIN1, DHTTYPE1);

#define DHTPIN2 15      // Digital pin connected to the DHT sensor
#define DHTTYPE2 DHT22  // DHT 1
DHT_Unified dht2(DHTPIN2, DHTTYPE2);

//#define ACTUATOR 15
//float tempDHT11 = 0.0;
//float tempDHT22 = 0.0;
//float humDHT11 = 0.0;
//float humDHT22 = 0.0;

//const int sensorPin6 = A6;   // seleccionar la entrada para el sensor
const int sensorPin6 = 12;
//int sensorValue1;  // variable que almacena el valor raw (0 a 1023)
//float value1;      // variable que almacena el voltaje (0.0 a 5.0)


uint32_t delayMS;

float getTemperature(DHT_Unified dht_sensor) {
  sensors_event_t event;
  dht_sensor.temperature().getEvent(&event);
  //dht2.temperature().getEvent(&event2);

  // tempDHT11 = event1.temperature;
  // tempDHT22 = event2.temperature;
  return event.temperature;

  // dht1.humidity().getEvent(&event1);
  // dht2.humidity().getEvent(&event2);
  // humDHT11 = event1.relative_humidity;
  // humDHT22 = event2.relative_humidity;
}

float getHumidity(DHT_Unified dht_sensor) {
  sensors_event_t event;
  dht_sensor.humidity().getEvent(&event);
  //dht2.temperature().getEvent(&event2);

  // tempDHT11 = event1.temperature;
  // tempDHT22 = event2.temperature;
  return event.relative_humidity;

  // dht1.humidity().getEvent(&event1);
  // dht2.humidity().getEvent(&event2);
  // humDHT11 = event1.relative_humidity;
  // humDHT22 = event2.relative_humidity;
}

float getRadiacion() {
  int sensorValue1 = analogRead(sensorPin6);           // realizar la lectura
  float value1 = fmap(sensorValue1, 0, 1023, 0.0, 5.0);  // cambiar escala a 0.0 - 5.0
  float tension_PPFD = value1 * 100;
  float PPFD = 5 * tension_PPFD;
  return PPFD;
}

// cambio de escala entre floats para la radiacion
float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup() {
  Serial.begin(9600);
  //pinMode(ACTUATOR, OUTPUT);
  // Initialize device.
  dht1.begin();
  dht2.begin();
  // Set delay between sensor readings based on sensor details.
  delayMS = 5000;
}

void loop() {

  // Delay between measurements.
  delay(delayMS);
  
  //getHumidity(dht1);
  //getHumidity(dht2);
  float rad = getRadiacion();
  Serial.print("Radiacion: ");
  Serial.println(getRadiacion());
  Serial.print("Temp dht11 ");
  Serial.print(getTemperature(dht1));
  Serial.print(" dht22 ");
  Serial.print(getTemperature(dht2));
  Serial.print(" hum dht11 ");
  Serial.print(getHumidity(dht1));
  Serial.print(" dht22 ");
  Serial.println(getHumidity(dht2));
  // Get temperature event and print its value.
}
