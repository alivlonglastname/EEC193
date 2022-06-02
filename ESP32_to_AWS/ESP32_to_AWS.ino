 #include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <WiFi.h>
#include "esp_camera.h"
#include <SoftwareSerial.h>
#include "Actuator.h"

#define MOTORPIN1 12
#define MOTORPIN2 13
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define ESP32CAM_PUBLISH_TOPIC   
#define ESP32CAM_RECOGNITION_TOPIC  "esp32/cam_0"
#define ESP32CAM_FACES_TOPIC        "faces"
#define LED_BUILTIN 4
const int bufferSize = 1024 * 23; // 23552 bytes

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(bufferSize);

int recognition = 0;

Actuator AC(MOTORPIN1, MOTORPIN2, 2000, 3000);
// class function defintions
// Constructor
Actuator::Actuator(int pin1, int pin2, int speedL, int speedU) {
    actuatorInit(pin1, pin2, speedL, speedU);
}
/// Actuator class start ///
void Actuator::forward(int duration){
    analogWrite(AIA, 0);
    analogWrite(AIB, 200);
    // Run for x seconds
    delay(duration);
    // Stop
    analogWrite(AIA, 0);
    analogWrite(AIB, 0);
}
void Actuator::backward(int duration) {
    analogWrite(AIA, 200);
    analogWrite(AIB, 0);
    // Run for x seconds
    delay(duration);
    // Stop
    analogWrite(AIA, 0);
    analogWrite(AIB, 0);
}

void Actuator::lock() {
    forward(LOCK);
    lockStatus = 1;
}

void Actuator::unlock() {
    backward(UNLOCK);
    lockStatus = 0;
}

void Actuator::init(bool lock) {
  backward(ZTOMAX);
  lockStatus = 0;
  if (lock) {
    forward(UNLOCK);
    lockStatus = 1;
    }
}

void Actuator::toggle() {
  if (lockStatus == 1) { // door is locked
    unlock();
  } else {
    lock();
  }
}

void Actuator::actuatorInit(int pin1, int pin2, int speedL, int speedU) {
    pinMode(pin1, OUTPUT);
    pinMode(pin2, OUTPUT);
    AIA = pin1;
    AIB = pin2;
    LOCK = speedL;
    UNLOCK = speedU;
}
/// Actuator class end ///

void connectAWS()
{
  delay(5000);
  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  client.setCleanSession(true);
  Serial.println("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    ESP.restart();
    return;
  }

  Serial.println("AWS IoT Connected!");
  // Subscribe to recognition topic
  client.subscribe("recognition/response");
  client.subscribe("esp32/lock");
  client.subscribe("esp32/fingerprint");
  // Set callback handler
  client.onMessage(callback);
}

String recognition_topic = "recognition/response";
String lock_topic = "esp32/lock";
String fingerprint_topic = "esp32/fingerprint";
// Callback handler, recieves 1 if image is recognized, 0 if not
void callback(String &topic, String &payload) {
  if (topic == recognition_topic) {
    recognition = payload.toInt();
  } else if (topic == lock_topic) {
    if (AC.lockStatus == 0) {
      Serial.println("Locking...");
    } else {
      Serial.println("Unlocking");
    }
    AC.toggle();
  } else if (topic == fingerprint_topic) {
    Serial.print("+");
  }
    
}

// Initialized Camera
void cameraInit(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA; // 640x480
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
    return;
  }
}

// Publishes image to AWS
void grabImage(){
  camera_fb_t * fb = esp_camera_fb_get();
  if(fb != NULL && fb->format == PIXFORMAT_JPEG && fb->len < bufferSize){
//    Serial.print("Image Length: ");
//    Serial.print(fb->len);
//    Serial.print("\t Publish Image: ");

    bool result = client.publish(ESP32CAM_RECOGNITION_TOPIC, (const char*)fb->buf, fb->len);
    client.publish(ESP32CAM_FACES_TOPIC, (const char*)fb->buf, fb->len);
    //Serial.println(result);

    if(!result){
      ESP.restart();
    }
  }
  esp_camera_fb_return(fb);
  delay(500);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  AC.init(false);
  cameraInit();
  connectAWS();
  digitalWrite(LED_BUILTIN, LOW);
}

int ON = 0;
void loop() {
  client.loop();
  if(client.connected()) {
    if (Serial.available()) {
      // ON is value recieved from master. If on is 1, turn on camera
      ON = Serial.parseInt();
    }

    if (ON == 1) {
      digitalWrite(LED_BUILTIN, HIGH);
      grabImage();
      digitalWrite(LED_BUILTIN, LOW);
    }
    
    if (recognition == 1) {
      // If image recognized, turn off camera, toggle door.
      recognition = 0;
      ON = 0;
      delay(50);
      AC.toggle();
      if (AC.lockStatus == 1) {
        Serial.print('-');
      } else {
        Serial.print('~');
      }
    }
    
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
  }
//    Serial.print("ON: ");
//    Serial.println(ON);
//    Serial.print("ref1: ");
//    Serial.println(ref1);
//    Serial.print("ref2: ");
//    Serial.println(ref2);
//    Serial.print("recognition: ");
//    Serial.println(recognition);
  delay(1000);
}
