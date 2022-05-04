// This is the sketch for Arduino Master
// Libraries
#include "Actuator.h"
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

#define MOTORPIN1 2
#define MOTORPIN2 3

SoftwareSerial esp32(6, 7);
Actuator AC(MOTORPIN1, MOTORPIN2, 2000, 2000);
// class function defintions
// Constructor
Actuator::Actuator(int pin1, int pin2, int speedL, int speedU) {
    actuatorInit(pin1, pin2, speedL, speedU);
}

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
}

void Actuator::unlock() {
    backward(UNLOCK);
}

void Actuator::init(bool lock) {
  forward(ZTOMAX);
  if (!lock) {
    backward(UNLOCK);
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

// Function declarations:

void Blink(int pin, int t, int MODE) {
  // must declare pin as output in setup first
  if (MODE == 1) {
    digitalWrite(pin, HIGH);
    delay(t / 2);
    digitalWrite(pin, LOW);
    delay(t / 2);
  } else {
    digitalWrite(pin, HIGH);
    delay(t / 4);
    digitalWrite(pin, LOW);
    delay(t / 4);
    digitalWrite(pin, HIGH);
    delay(t / 4);
    digitalWrite(pin, LOW);
    delay(t);
  }
}

void esp32init() {
  esp32.begin(115200);
  esp32.println("Arduino awake, awaiting connection status to blynk...");
  esp32.print("-c-");
  delay(100);
  // Wait until ESP32 responds
  while(!esp32.available()) {
    Blink(13, 500, 1);
  }
  if (esp32.available()) {
    if (esp32.parseInt() < 0) {
      Serial.println("ESP32 not connected to Blynk, stalling");
      while(true);
    } else {
      Serial.println("ESP32 connected to Blynk Succesfully! Awaiting Data!");
      esp32.print("-r-");
    }
  }
}

void setup() {
  Serial.begin(9600);
  // Pin definitions - must declare 13 before calling blink
  pinMode(13, OUTPUT);
  esp32init();
  AC.init(false);
  
}

void loop() {
  if (esp32.available()) {
    char c = esp32.read();
    if (c == 'L') {
      AC.lock();
    } else if (c == 'U') {
      AC.unlock();
    }
  }
  // put your main code here, to run repeatedly:
  
}
