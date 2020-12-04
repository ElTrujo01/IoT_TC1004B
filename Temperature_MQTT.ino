#include <ArduinoJson.h>

#include <Arduino_JSON.h>

StaticJsonDocument<200> doc;

const int dht_pin = 2; //pin d4
const int led = 4; //pin d2
const int fan_out = 0; //pin d3

#include "secrets.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>  
#include "DHT.h"
#include <Adafruit_Sensor.h>

const int dht_tipo = DHT11;

float dht_humedad;
float dht_temperatura;

DHT dht(dht_pin, dht_tipo);

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
const char* mqtt_server = "broker.mqtt-dashboard.com";

IPAddress ip;
char ipno[26] ;
int statusActuador;
const char* sensor = "DHT11";

const char* topico_temperatura = "DHT11/Temperatura";
const char* topico_dispositivo = "DHT11/Dispositivo";
const char* topico_casa = "DHT11/Casa";
const char* topico_RH = "DHT11/RH";
const char* topico_status = "DHT11/Status";
const char* topico_sensorType = "DHT11/SensorType";
const char* topico_datos = "node/data";
char sTopicoTemperatura[50];
char sTopicoDispositivo[50];
char sTopicoCasa[50];
char sTopicoRH[50];
char sTopicoStatus[50];
char sTopicoSensorType[50];
char sTopicoDatos[150];

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
#define MSG_BUFFER_SIZE2  (50)
char msg2[MSG_BUFFER_SIZE2];
#define MSG_BUFFER_SIZE3  (50)
char msg3[MSG_BUFFER_SIZE3];
#define MSG_BUFFER_SIZE4  (50)
char msg4[MSG_BUFFER_SIZE4];
#define MSG_BUFFER_SIZE5  (50)
char msg5[MSG_BUFFER_SIZE5];
#define MSG_BUFFER_SIZE6  (50)
char msg6[MSG_BUFFER_SIZE6];
#define MSG_BUFFER_SIZE7  (150)
char msg7[MSG_BUFFER_SIZE7];
int value = 0;
int value2 = 0;

void setup_wifi() { // -> conexion WiFi
  delay(10);
  Serial.println();    Serial.print("Connecting to ");   Serial.println(ssid);
  WiFi.mode(WIFI_STA); WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println(""); Serial.println("WiFi connected"); Serial.print("IP address: "); Serial.println(WiFi.localIP());

  ip = WiFi.localIP();
  
}

void setup_mqtt() { // -> conexion al broker
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  if (!client.connected()) {
    reconnect();
  }
}

void callback(char* topic, byte* payload, unsigned int length) { // -> funcion callback
  Serial.print("Message arrived ["); Serial.print(topic); Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() { // -> reconexion
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      /*client.publish(topico_temperatura, sTopicoTemperatura);
      client.publish(topico_RH, sTopicoRH); 
      client.publish(topico_dispositivo, sTopicoDispositivo);
      client.publish(topico_casa, sTopicoCasa);   
      client.publish(topico_status, sTopicoStatus);  
      client.publish(topico_sensorType, sTopicoSensorType);
      client.publish(topico_datos, sTopicoDatos);*/
     
    } else {
      Serial.print("failed, rc="); Serial.print(client.state()); Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void conectarMQTT() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
           
  unsigned long now = millis();
  if (now - lastMsg > 3000) {
    lastMsg = now;
    ++value;
    
    Serial.print("Temperatura ambiental: "); Serial.println(sTopicoTemperatura);
    Serial.print("Humedad relativa: "); Serial.println(sTopicoRH);
    Serial.print("ID del dispositivo: "); Serial.println(sTopicoDispositivo);
    Serial.print("ID de la casa: "); Serial.println(sTopicoCasa);
    Serial.print("Status del Actuador: "); Serial.println(sTopicoStatus);
    Serial.print("Tipo de sensor: "); Serial.println(sTopicoSensorType);
    Serial.print("Datos: "); Serial.println(sTopicoDatos);
    /*client.publish(topico_temperatura, sTopicoTemperatura);
    client.publish(topico_RH, sTopicoRH);
    client.publish(topico_dispositivo, sTopicoDispositivo);
    client.publish(topico_casa, sTopicoCasa);
    client.publish(topico_status, sTopicoStatus);
    client.publish(topico_sensorType, sTopicoSensorType);*/
    client.publish(topico_datos, sTopicoDatos);
  }
}

// =================================================================
// ----- Funcion que lee temperatura y humedad con el sensor DHT11
void medirTemperatura() {
  // Lee los valores de humedad y temperatura
  float dht_humedad = dht.readHumidity(); 
  float dht_temperatura = dht.readTemperature();

  if(dht_temperatura > 27){
    digitalWrite(fan_out,HIGH); 
    digitalWrite(led,HIGH);
    statusActuador = 1;
  }
  else{
    digitalWrite(fan_out,LOW);
    digitalWrite(led,LOW);
    statusActuador = 0;
  }

  // Envia las lecturas al monitor serial
  Serial.println();
  Serial.print("Humedad ");
  Serial.print(dht_humedad);
  Serial.println(" %");
  Serial.print("Temperatura: ");
  Serial.print(dht_temperatura);
  Serial.println("ºC");

  char id_disp[] = "Alondra2_2";

  doc["casa"] = ssid;
  doc["dispositivo"] = id_disp;
  doc["temperatura"] = dht_temperatura;
  doc["humedad"] = dht_humedad;
  doc["stats"] = statusActuador;

  char output[100];

  serializeJson(doc, output);
    
  snprintf (sTopicoTemperatura, MSG_BUFFER_SIZE, "%f",dht_temperatura);
  snprintf (sTopicoRH, MSG_BUFFER_SIZE4, "%f",dht_humedad);
  snprintf (sTopicoDispositivo, MSG_BUFFER_SIZE2, "%s",id_disp);
  snprintf (sTopicoCasa, MSG_BUFFER_SIZE3, "%s",ssid);
  snprintf (sTopicoStatus, MSG_BUFFER_SIZE5, "%i", statusActuador);
  snprintf (sTopicoSensorType, MSG_BUFFER_SIZE6, "%s",sensor);
  snprintf (sTopicoDatos, MSG_BUFFER_SIZE7, "%s",output);
  
  Serial.println();
}
// =================================================================
void setup() {
  Serial.begin(9600);  // Iniciar consola
  
  // ----- Abrir Wifi y MQTT
  setup_wifi();
  setup_mqtt();
  
  // ----- Inicializacion sensor temperatura - humedad
  Serial.println(F("Temperature & RH test!"));

  dht.begin(); // Inicializa sensor
  pinMode(fan_out, OUTPUT); // Inicializa pin de salida
  pinMode(led, OUTPUT);

}
void loop() {
   medirTemperatura();
   conectarMQTT();
   delay(600000);
}
