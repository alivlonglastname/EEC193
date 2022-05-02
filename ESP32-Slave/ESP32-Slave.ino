// Defintions
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPLfgUI_eFF"
#define BLYNK_DEVICE_NAME "LED ON"
#define APP_DEBUG
#define WAVCODE           "-m-"
// Blynk Defintions
#define BLYNK_AUTH_TOKEN "7gNcZrMwi68Zla81GB01Jn6RK6FVf2pG"
char ssid[] = "Moderately Fast WiFi";
char pass[] = "veryhardpassword";
String msg;
String ping = "Ping from Blynk";
char buf;
char readbuf[4];
int startWav = 0;
int wavindex = 0;
// Library Declarations
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>



// Pin Declarations:
// OLED:
// On an arduino UNO:       A4(SDA), A5(SCL)
// on ESP32: 22(SCL), 21(SDA)
// MIC:
// Arduino: TX22, RX21

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Functions

void OLED_start() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Display Ready!");
  display.display();
}

void debug(String message) {
  if (display.getCursorY() > 56) {
    display.clearDisplay();
    display.setCursor(0, 0);
  }
  int L = message.length();
  for (int i = 0; i < L; i++) {
    display.print(message[i]);
    if (i > 128) {
      display.println();
    }
  }
  display.display();
}

void push_back(char *buf, char d) {
  int L = strlen(buf);
  for (int i = 0; i <= L - 1; i++) {
    buf[i] = buf[i+1];
  }
  buf[L-1] = d;
}


BLYNK_WRITE(V0)
{
  debug(ping);
}

void wave_viewer(int x, byte Data) {
  int newData = Data;
  // Data is of size 256 and ranges from 0 - 255. This portion of code makes it range from -128 to 128
  if (x == 127) {display.clearDisplay();}
  // convert unsigned hex to signed hex
  if (newData >= 128) {newData -= 256;}
  display.drawPixel(x, newData / 8 + 32, SSD1306_WHITE);
  display.display();
    
}

void setup() {
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000);
  OLED_start();
  debug("Connecting to WiFi...\n");
  delay(1000);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  if (WiFi.status() == WL_CONNECTED) {
      debug("Connected!\n");
    } else {
      debug("Failed\n");
    }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  
}

void loop() {
  Blynk.run();
  while (Serial.available()){
    
    if (startWav == 1) { // Starting Wave viewer
      Serial.println("startWav == 1");
      display.clearDisplay();
      startWav = 0;
      int xpos = 0;
      delay(1000);
      while(Serial.available()) {
        buf = Serial.read();
        wave_viewer(++xpos % 128, (unsigned char)buf);
      }
    }
    buf = Serial.read();
    if (wavindex == 3) {
      push_back(readbuf, buf);
      } else {
        readbuf[wavindex] = buf; // Update readbuf
        wavindex++;
      }
      // idk it wouldnt work any other way dont ask questions
    if (readbuf[0] == '-' && readbuf[1] == 'm' && readbuf[2] == '-') {
      startWav = 1;
    }
    //Serial.println(readbuf);
    msg += buf;
  }
  if (!Serial.available() && msg.length() > 0) {
    debug(msg);
    msg = "";
  }
}
