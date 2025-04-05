
## Requisitos previos

1. **Hardware**:
   - ESP32
   - Sensores compatibles (DHT11, DHT22, etc.)
2. **Software**:
   - Arduino IDE
   - Docker

## Configuración

1. **ESP32**:
   - Configura las credenciales Wi-Fi en `env.h`.
   - Sube el código desde `code.ino` al ESP32.

2. **FIWARE y Node-RED**:
   - Usa `docker-compose.yml` en `fiwareNodeRed/` para iniciar los contenedores:
     ```bash
     docker-compose up -d
     ```
   - Accede a Node-RED en [http://localhost:1880](http://_vscodecontentref_/3).

3. **Flujos de Node-RED**:
   - Importa los flujos desde [flows.json](http://_vscodecontentref_/4).

## Uso

1. **Captura de datos**:
   - El ESP32 lee datos de los sensores y los envía a FIWARE Orion.
2. **Procesamiento**:
   - Node-RED procesa los datos y los muestra en dashboards.
3. **Visualización**:
   - Accede a los dashboards en Node-RED para monitorear los datos en tiempo real.

## Ejemplo de entidad en FIWARE

```json
{
  "id": "EstacionAIOT-SmartHome",
  "type": "EstacionAIOT",
  "sDHT11_Temperatura": {
    "type": "float",
    "value": 23.5
  },
  "sDHT11_Humedad": {
    "type": "float",
    "value": 45.2
  },
  "sDHT22_Temperatura": {
    "type": "float",
    "value": 24.8
  },
  "sDHT22_Humedad": {
    "type": "float",
    "value": 50.1
  },
  "sSuelo_Agua": {
    "type": "float",
    "value": 12.3
  },
  "sSuelo_Dielectricidad": {
    "type": "float",
    "value": 30.5
  },
  "sSuelo_Conductividad": {
    "type": "float",
    "value": 40.7
  },
  "sSuelo_Temperatura": {
    "type": "float",
    "value": 22.1
  },
  "sRadiacion": {
    "type": "float",
    "value": 300.0
  }
}