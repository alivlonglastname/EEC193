#include <Adafruit_Fingerprint.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
int enroll = 0;
uint8_t id;
uint8_t numPrints = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);

void setup()
{
  Serial.begin(9600);
  Serial2.begin(115200);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds
  Serial.println("Display started");
  OLED_start();
  Serial.println("\n\nAdafruit finger detect test");
  
  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();
  numPrints = finger.templateCount + 1;
  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please enroll finger.");
    while (!getFingerprintEnroll());
  }
  
  else {
    display.println("Waiting for valid finger...");
    display.display();
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}
char c;
void loop()                     // run over and over again
{
  while (Serial2.available()) {
    c = Serial2.read();
    if (c == '+') {
      enroll = 1;
      break;
    } else if (c == '-') {
      debug("locking");
      break;
    } else if (c == '~') {
      debug("unlocking");
      break;
    }
    
    debug(String(c));
  }
  if (enroll == 1) {
    while(getFingerprintEnroll() != true);
    enroll = 0;
  } else {
    getFingerprintIDez();
  }
  delay(50);            //don't ned to run this at full speed.
}

void OLED_start() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Display Ready!");
  display.display();
  delay(2000);
  display.clearDisplay();
  
}


uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

uint8_t getFingerprintEnroll() {
  
  id = numPrints;
  int p = -1;
  debug("please scan finger to enroll\n");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      debug("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      break;
    default:
      debug("error, try again\n");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      debug("...ok\n");
      break;
    default:
      debug("error, try again\n");
      return p;
  }

  debug("Remove finger\n");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  p = -1;
  debug("Place same finger again\n");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      debug("Image taken\n");
      break;
    case FINGERPRINT_NOFINGER:
      break;
    default:
      debug("error, try again\n");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      debug("...ok\n");
      break;
    default:
      debug("error, try again\n");
      return p;
  }

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    debug("Success!");

  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    debug("error, try again\n");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    debug("Fingerprints not matching\n");
    return p;
  } else {
    debug("error, try again\n");
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    debug("Fingerprint enrolled successfully");
  } else {
    debug("error, try again\n");
    return p;
  }
  numPrints++;
  return true;
}

// returns -1 if failed, otherwise returns ID #
int waiting = 0; // used to print 1 message until a finger is scanned, repeat
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p == FINGERPRINT_NOFINGER) {
    if (waiting == 0) {
      debug("scan finger to lock/unlock\n");
      waiting = 1;
    }
    return -1;
  }
  if (p != FINGERPRINT_OK)  {
    debug("Error scanning\n");
    return -1;
    }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)    {
    debug("Error scanning\n");
    return -1;
    }
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)    {
    debug("Not recognized\n");
    return -1;
    }

  // found a match!
  debug("Found a print match!\nPlease Look at Camera\n");
  waiting = 0;
  Serial2.print(1);
  delay(10000);
  Serial2.print(0);
  return finger.fingerID;
}

// Prints message to OLED
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
