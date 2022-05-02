// Library Declarations
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

File WAV1;
// Definitions

// Declarations
SoftwareSerial ESP32(2, 3);

// Functions
void SD_setup() {
  if (!SD.begin(4)) {
    ESP32.println("SD initialization failed!");
    while (1);
  }
  ESP32.println("SD initialization done.");
}
void setup() {
  Serial.begin(9600);
  ESP32.begin(115200);
  SD_setup();
  WAV1 = SD.open("sine.wav");
  ESP32.print("-m-");
  WAV1.seek(36);
  // put your setup code here, to run once:

}
String msg;
byte d = 0x00;
char j;
void loop() {
  delay(1000);
//  Serial.print("Sending: ");
  for (int i = 0; i < 128; i++) {
    j = WAV1.read();
    ESP32.print(j);
//  delay(100);
//    Serial.print(j, HEX);
//    Serial.print(", ");
    
  }
//  Serial.println();
  
}
