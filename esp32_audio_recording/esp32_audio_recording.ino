#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
 
#define PUBTOPIC   "esp32/out"
#define SUBTOPIC   "esp32/in"

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

// timer stuff
volatile int interruptCounter;
int totalInterruptCounter;
uint8_t audioArray[512];
int audioIndex = 0;
int mic_pin = 18;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void awsInit()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
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
 
void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["humidity"] = "hi";
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
 
  client.publish(PUBTOPIC, jsonBuffer);
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

 // interrupt function
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
//  if (audioIndex == 512) {
//    Serial.println("publishing");
//    audioIndex = 0;
//    client.publish(PUBTOPIC, audioArray, 512);
//  }
 // audioArray[++audioIndex] = analogRead(mic_pin);
  portEXIT_CRITICAL_ISR(&timerMux);
 
}

void timerInit(int period) {
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, period, true);
  timerAlarmEnable(timer);
}
 
void setup()
{
  Serial.begin(115200);
  timerInit(1000); // in microseconds, 1000000 = 1 second
  awsInit(); // connect to aws

}
 
void loop()
{
  client.loop();
  client.publish(PUBTOPIC, "HELLO");
  delay(3000);
  // so far this is working code of pubsub jesus chris
}
