#include <ArduinoJson.h>

#include <Arduino_JSON.h>

StaticJsonDocument<200> doc;

int pos = 0;    // variable to store the servo position
int posFinal = 10;
int posInicial = 0;

/*Aqui se configuran los pines donde debemos conectar el sensor*/
const int Trigger = 5;   //Pin digital 2 para el Trigger del sensor
const int Echo = 14;   //Pin digital 0 para el Echo del sensor
const int led = 4;
const int servo = 2; //D4
const int alerta = 0; //D3

long dis;
long tiem;
int servoVer = 0;
int ledVer = 0;

#include "secrets.h"
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>  

Servo myservo;  // create servo object to control a servo

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
const char* mqtt_server = "broker.mqtt-dashboard.com";

IPAddress ip;
char ipno[26] ;
int statusActuador;
const char* sensor = "Ultrasonido";

const char* topico_distancia = "Ultrasonido/Distancia";
const char* topico_dispositivo = "Ultrasonido/Dispositivo";
const char* topico_casa = "Ultrasonido/Casa";
const char* topico_status = "Ultrasonido/Status";
const char* topico_sensorType = "Ultrasonido/SensorType";
const char* topico_datos = "node/data";
char sTopicoDistancia[50];
char sTopicoDispositivo[50];
char sTopicoCasa[50];
char sTopicoStatus[50];
char sTopicoSensorType[50];
char sTopicoDatos[120];

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
#define MSG_BUFFER_SIZE6  (120)
char msg6[MSG_BUFFER_SIZE6];
int value = 0;

void setup_wifi() { // -> conexion WiFi
  digitalWrite(alerta, HIGH);
  delay(2000);
  digitalWrite(alerta, LOW);
  delay(10);
  Serial.println();    Serial.print("Connecting to ");   Serial.println(ssid);
  WiFi.mode(WIFI_STA); WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(alerta, HIGH);
    delay(500);
    digitalWrite(alerta, LOW);
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
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      //REDV client.subscribe(topico_salida);   //OJO Quitar el comentario para hacerlo suscribe
      /*client.publish(topico_distancia, sTopicoDistancia);
      client.publish(topico_dispositivo, sTopicoDispositivo);
      client.publish(topico_casa, sTopicoCasa);    
      client.publish(topico_status, sTopicoStatus);  
      client.publish(topico_sensorType, sTopicoSensorType);
      client.publish(topico_datos, sTopicoDatos);*/
      // ... and resubscribe
      //client.subscribe("inTopic");
     
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
    
    Serial.print("Distancia: "); Serial.println(sTopicoDistancia);
    Serial.print("ID del dispositivo: "); Serial.println(sTopicoDispositivo);
    Serial.print("ID de la casa: "); Serial.println(sTopicoCasa);
    Serial.print("Status del Actuador: "); Serial.println(sTopicoStatus);
    Serial.print("Tipo de sensor: "); Serial.println(sTopicoSensorType);
    Serial.print("Datos: "); Serial.println(sTopicoDatos);
    /*client.publish(topico_distancia, sTopicoDistancia);
    client.publish(topico_dispositivo, sTopicoDispositivo);
    client.publish(topico_casa, sTopicoCasa);
    client.publish(topico_status, sTopicoStatus);
    client.publish(topico_sensorType, sTopicoSensorType);*/
    client.publish(topico_datos, sTopicoDatos);
  }
}

// =================================================================
// ----- Funcion que lee temperatura y humedad con el sensor DHT11
void medirDistancia() {
  digitalWrite(Trigger,LOW);//recibimiento del pulso.
  delayMicroseconds(5);
  digitalWrite(Trigger, HIGH);//envió del pulso.
  delayMicroseconds(10);
  tiem=pulseIn(Echo, HIGH);//fórmula para medir el pulso entrante.
  dis= tiem/59;//fórmula para calcular la distancia del sensor ultrasónico.
  
  if(dis > 0 && dis < 60){ //comparativo para la alarma se ingresa la distancia en la que encenderá o apagara.
    digitalWrite(led, HIGH);
    ledVer = 1;
    statusActuador = 1;
  }
  else{
    digitalWrite(led, LOW);
    ledVer = 0;
    statusActuador = 0;
  }

  if(dis < 60 && ledVer == 1 && servoVer == 0){ //comparativo para la alarma se ingresa la distancia en la que encenderá o apagara.
    for (pos = posInicial; pos <= posFinal; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    for (pos = posFinal; pos >= posInicial; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    for (pos = posInicial; pos <= posFinal; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    servoVer = 1;
  }
  else if (dis > 60 && ledVer == 0 && servoVer == 1) {
    for (pos = posInicial; pos <= posFinal; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    for (pos = posFinal; pos >= posInicial ; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    for (pos = posInicial; pos <= posFinal; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    servoVer = 0;
  }

  Serial.println();
  Serial.print("LA DISTANCIA MEDIDA ES:");
  Serial.print(dis);
  Serial.print("cm");

  float value = dis;

  if(value>400){
    value = 400;
  }

  char id_disp[] = "";

  doc["casa"] = ssid;
  doc["dispositivo"] = id_disp;
  doc["distancia"] = value;
  doc["stats"] = statusActuador;

  char output[100];

  serializeJson(doc, output);
  //Serial.println(output);
    
  snprintf (sTopicoDistancia, MSG_BUFFER_SIZE, "%i",value);
  snprintf (sTopicoDispositivo, MSG_BUFFER_SIZE2, "%s", id_disp);
  snprintf (sTopicoCasa, MSG_BUFFER_SIZE3, "%s",ssid);
  snprintf (sTopicoStatus, MSG_BUFFER_SIZE4, "%i",statusActuador);
  snprintf (sTopicoSensorType, MSG_BUFFER_SIZE5, "%s",sensor);
  snprintf (sTopicoDatos, MSG_BUFFER_SIZE6, "%s",output);
  
  Serial.println();
}
// =================================================================
void setup() {
  Serial.begin(9600);  // Iniciar consola
  
  // ----- Abrir Wifi y MQTT
  setup_wifi();
  setup_mqtt();
  
  // ----- Inicializacion sensor ultrasonido
  Serial.println(F("Ultrasound test!"));

  pinMode(Trigger, OUTPUT); //salida del pulso generado por el sensor ultrasónico
  pinMode(Echo, INPUT);//entrada del pulso generado por el sensor ultrasónico
  pinMode(led, OUTPUT);//alarma de la distancia(encenderá el led)
  pinMode(alerta, OUTPUT); //alerta de conexion
  myservo.attach(servo);  // attaches the servo on pin D4 (2) to the servo object

}
void loop() {
   medirDistancia();
   conectarMQTT();
   delay(600000);
}
