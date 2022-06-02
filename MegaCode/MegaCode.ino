#include <SPI.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"
#define PUBTOPIC   "esp32/out"
#define SUBTOPIC   "esp32/in"
WiFiClientSecure mqtt;
PubSubClient client(mqtt);
// wifi data in arduino_secrets.h
char ssid[] = WIFI_SSID;        // network SSID (name)
char pass[] = WIFI_PASS;        //  network password

int status = WL_IDLE_STATUS;

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}
void awsInit()
{
  WiFi.hostname("mkr1000");

  if (WiFi.status() == WL_NO_SHIELD) {
    while (true);       // don't continue
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection
    delay(1000);
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  mqtt.setCACert(AWS_CERT_CA);
  mqtt.setCertificate(AWS_CERT_CRT);
  mqtt.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(SUBTOPIC);
 
  Serial.println("AWS IoT Connected!");
}

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}


void setup() {
  Serial.begin(9600);
  awsInit();
}


void loop() {

  delay(1500);
}
