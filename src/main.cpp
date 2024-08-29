#include <Arduino.h>
#include <WiFi.h> //Librería que permite incluir las funciones de conexión a internet.
#include <Wire.h> //Librería que permite incluir las funciones de comunicación I2C.
#include "ClosedCube_HDC1080.h" //Librería que permite incluir las funciones del sensor de temperatura y humedad.
#include <TinyGPSPlus.h> //Librería que permite incluir las funciones del GPS.

//Declaración de funciones
void prunning();
float temperatura(int nro_temps);
float humedad(int nro_hums);
float* gpsData(int nro_data);
static void smartDelay(unsigned long ms);

//Wifi
const char* host = "54.80.92.106"; //direccion ip publica del servidor (maquina virtual)
const uint16_t port = 80; //puerto por el cual me voy a conectar
const char* ssid = "UPBWiFi"; //Nombre de la red upb

//WiFiClient client; //Cliente wifi para admin la comunicación

//Temperatura y Humedad
ClosedCube_HDC1080 sensor;

//GPS
TinyGPSPlus gps;
HardwareSerial GPS(1);
static const uint32_t GPSBaud = 9600;

void setup() {
  Serial.begin(115200); //Iniciar la comunicación serial (me conecto a capa 2)
  Wire.begin(0, 4); //Iniciar la comunicación I2C (sda, scl)

  sensor.begin(0x40);
  delay(20);

  Serial1.begin(9600, SERIAL_8N1, 34, 12); //Comunicación con el GPS (baudrate, protocolo, rx, tx)


  WiFi.mode(WIFI_STA); //Como se utiliza el wifi, su utiliza como cliente (tambien hay modo router)
  WiFi.begin(ssid); //Conectarse a la red upb.

  while (WiFi.status() != WL_CONNECTED) { //bucle infinito que no pasa de ahi hasta que se conecte
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(5000); //Mandar cada 5 seg un paquete

  //Medición de temperatura y humedad
  Serial.printf("Temperatura: %.2f C\n", temperatura(3));
  Serial.printf("Humedad: %.2f %%\n", humedad(3));

  //Medición de GPS
  smartDelay(1000);
  Serial.printf("Latitud: %f\n", gpsData(3)[0]);
  Serial.printf("Longitud: %f\n", gpsData(3)[1]);

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
    temperaturas[i] = temp;
    delay(100);
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
    humedades[i] = hum;
    delay(100);
  }

  float promedio = prunning(humedades, nro_hums);
  return promedio;
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (GPS.available())
      gps.encode(GPS.read());
  } while (millis() - start < ms);
}

float* gpsData(int nro_data){
  gps.encode(GPS.read());
  smartDelay(1000);

  float latitud, longitud;
  float* data = new float[2];

  //Calcula la latitud y longitud n veces y guarda los ultimos valores (prunning).
  for (int i = 0; i < nro_data; i++)
  {
    latitud = gps.location.lat();
    longitud = gps.location.lng();
    smartDelay(100);
  } 

  data[0] = latitud;
  data[1] = longitud;

  return data;
}