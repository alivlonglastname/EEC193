// Defintions
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPLfgUI_eFF"
#define BLYNK_DEVICE_NAME "LED ON"
#define APP_DEBUG

// Blynk Defintions
#define BLYNK_AUTH_TOKEN "7gNcZrMwi68Zla81GB01Jn6RK6FVf2pG"
char ssid[] = "Moderately Fast WiFi";
char pass[] = "veryhardpassword";
String msg;
char buf;
char readbuf[4];
int bufIndex = 0;
int cStatus = -1; // connection status
bool READY = false;
// Library Declarations
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SoftwareSerial.h>

SoftwareSerial arduino(16, 17);



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
  int val = param.asInt();
  if (val == 1 && READY) {
    arduino.print('L');
    debug("Locking; ");
  } else if (val == 0 && READY){
    arduino.print('U');
    debug("Unlocking; ");
  }
}

void setup() {
  Serial.begin(115200);
  arduino.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.display();
  delay(2000);
  OLED_start();
  debug("Connecting to WiFi...\n");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  if (WiFi.status() == WL_CONNECTED) {
      debug("Connected!\n");
      cStatus = 1;
    } else {
      debug("Failed\n");
    }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  
}

void loop() {
  Blynk.run();
  while (arduino.available()){
    delay(5);
    buf = arduino.read();
    if (bufIndex == 3) {
      push_back(readbuf, buf);
      } else {
        readbuf[bufIndex] = buf; // Update readbuf
        bufIndex++;
      }
    // idk it wouldnt work any other way dont ask questions // Trust me
    if (readbuf[0] == '-' && readbuf[1] == 'c' && readbuf[2] == '-') {
      debug("sending cstatus");
      arduino.print(cStatus);
      // reset readbuf
    } else if (readbuf[0] == '-' && readbuf[1] == 'r' && readbuf[2] == '-') {
      debug("Arduino Ready! Awaiting Blynk\n");
      READY = true;
      // switch behavior, start sending info from blynk
    }
    //Serial.println(readbuf);
    msg += buf;
  }
  if (!arduino.available() && msg.length() > 0) {
    debug(msg);
    msg = "";
  }
  delay(1000);
}
