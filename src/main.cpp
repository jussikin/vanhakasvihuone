#include <Arduino.h>
#include <MLEDScroll.h>
#include <config.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>

#define MOVEMENT_PIN D4
#define TIMOUT 160
#define SKIPS 10
#define DIRA_PIN D8
#define DIRB_PIN D7

char ssid[] = WIFIHOTSPOT;
char pw[] = WIFIKEY;
String status = "undefined";

WiFiClient espClient;
PubSubClient client(espClient);
Ticker sekuntiTikuttaja;
int secudejaJaSusia = 0;
int skip =0;

void tickUp(){
  secudejaJaSusia++;
  if(secudejaJaSusia>TIMOUT){
    ESP.reset();
  }
}



char payloadArray[20];
void sendStatus(){
   status.toCharArray(payloadArray, 20);
   client.publish(WINDOWWANHA,payloadArray);
}

void callback(char* topic, byte* payload, unsigned int length) {
 secudejaJaSusia=0;
 if(topic==COMMANDWANHA){
   if(payload[0]=1 && (status=="undefined" || status=="closed")){
     status="opening";
     sendStatus();
     setDirA();
     delay(30*1000);
     status="open";
     sendStatus();
     stop();
   }

   if(payload[0]=0 && (status=="undefined" || status=="open")){
     status="closing";
     sendStatus();
     setDirB();
     delay(30*1000);
     status="closed";
     sendStatus();
     stop();
   }

 }
}

void setupWifi(){
    WiFi.begin(WIFIHOTSPOT, WIFIKEY);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
    client.setServer(MQTTSERVER, 1883);
    client.setCallback(callback);
}

void stop(){
  digitalWrite(DIRA_PIN,LOW);
  digitalWrite(DIRB_PIN,LOW);
}

void setDirA(){
  digitalWrite(DIRB_PIN,HIGH);
  digitalWrite(DIRA_PIN,LOW);
}

void setDirB(){
  digitalWrite(DIRB_PIN,LOW);
  digitalWrite(DIRA_PIN,HIGH);
}


void setup() {
    sekuntiTikuttaja.attach(1,tickUp);
    setupWifi();
    pinMode(DIRA_PIN,OUTPUT);
    pinMode(DIRB_PIN,OUTPUT);
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(CBWANHA);
      client.subscribe(COMMANDWANHA);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
   if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if(skip==0){
   sendStatus();
   skip = SKIPS;
  }
  skip--;
  delay(1000);
  client.loop();
}