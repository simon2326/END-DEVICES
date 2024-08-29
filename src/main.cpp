#include <Arduino.h>
#include <WiFi.h> //Librería que permite incluir las funciones de conexión a internet.
#include <Wire.h> //Librería que permite incluir las funciones de comunicación I2C.
#include "ClosedCube_HDC1080.h" //Librería que permite incluir las funciones del sensor de temperatura y humedad.
#include <TinyGPSPlus.h> //Librería que permite incluir las funciones del GPS.

//Declaración de funciones
void prunning();
float temperatura(int nro_temps);
float humedad(int nro_hums);
float* gpsData();
void bundling(float temp, float hum, float* gps);
static void smartDelay(unsigned long ms);

//Wifi
const char* host = "10.38.32.137"; //direccion ip publica del servidor (maquina virtual)
const uint16_t port = 80; //puerto por el cual me voy a conectar
const char* ssid = "UPBWiFi"; //Nombre de la red upb
WiFiClient client; //Cliente wifi para admin la comunicación

//Temperatura y Humedad
ClosedCube_HDC1080 sensor;

//GPS
TinyGPSPlus gps;

void setup() {
  Wire.begin(0, 4); //Iniciar la comunicación I2C (sda, scl)
  smartDelay(100);

  sensor.begin(0x40);
  smartDelay(20);

  Serial.begin(115200); //Iniciar la comunicación serial (me conecto a capa 2)
  Serial1.begin(9600, SERIAL_8N1, 34, 12); //Comunicación con el GPS (baudrate, protocolo, rx, tx)
  smartDelay(20);

  WiFi.mode(WIFI_STA); //Como se utiliza el wifi, su utiliza como cliente (tambien hay modo router)
  WiFi.begin(ssid); //Conectarse a la red upb.

  while (WiFi.status() != WL_CONNECTED) { //bucle infinito que no pasa de ahi hasta que se conecte
    smartDelay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("\n");

  smartDelay(100);
}

void loop() {

  float temp = temperatura(3);
  float hum = humedad(3);
  float* gps = gpsData();
  
  //Medición de temperatura y humedad
  Serial.printf("Temperatura: %.2f C\n", temp);
  Serial.printf("Humedad: %.2f %%\n", hum);

  //Medición de GPS
  Serial.printf("Latitud: %f\n", gps[0]);
  Serial.printf("Longitud: %f\n", gps[1]);

  //Envío de datos al servidor
  bundling(temp, hum, gps);

  for (int i = 0; i < 18; i++){
    smartDelay(500); //La suma de los SmartDelay es 9 segundos, calculando la temperatura, humedad y posición me gasto 1 segundo.
  }
}

//Prunning
float prunning(float datos[], float len){
  
  float total = 0;
  for (int i = 0; i < len; i++)
  {
    total += datos[i];
  }
  return (total/len);
  
}

//Obtener temperatura (prunning incluido)
float temperatura(int nro_temps){

  float temp = 0;
  float temperaturas[nro_temps] = {};

  for (int i = 0; i < nro_temps; i++)
  {
    temp = sensor.readTemperature();
    smartDelay(100);
    temperaturas[i] = temp;
  }

  float promedio = prunning(temperaturas, nro_temps);
  return promedio;
}

//Obtener humedad (prunning incluido)
float humedad(int nro_hums){

  float hum = 0;
  float humedades[nro_hums] = {};

  for (int i = 0; i < nro_hums; i++)
  {
    hum = sensor.readHumidity();
    smartDelay(100);
    humedades[i] = hum;
  }

  float promedio = prunning(humedades, nro_hums);
  return promedio;
}

//Obtener datos del GPS
float* gpsData(){
  smartDelay(400);
  float* data = new float[2];

  if(gps.location.isUpdated()){
    data[0] = gps.location.lat();
    data[1] = gps.location.lng();
  }
  return data;
}

//Envío de datos
void bundling(float temp, float hum, float* gps){

  if(client.connect(host, port))
  {
    Serial.println("Conectado al servidor");
    String json = "{\"id\": \"point12\", \"lat\": " + String(gps[0], 6) + ", \"lon\": " + String(gps[1], 6) + ", \"temperatura\": " + String(temp, 2) + ", \"humedad\": " + String(hum, 2) + "}";

    client.println("POST /update_data HTTP/1.1"); 
    client.println("Host: 10.38.32.137");
    client.println("Content-Type: application/json");
    client.println("Content-Length: " + String(json.length()));
    client.println("");
    client.println(json);

    Serial.println("Datos enviados\n");
  }
  else
  {
    Serial.println("Error al conectar al servidor");
  }
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (Serial1.available())
      gps.encode(Serial1.read());
  } while (millis() - start < ms);
}