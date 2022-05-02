#define BLYNK_TEMPLATE_ID "TMPLfgUI_eFF"
#define BLYNK_DEVICE_NAME "LED ON"
#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial

#define APP_DEBUG
#define RED 22
#define GREEN 23

#include "C:\Users\alihu\Documents\Arduino\libraries\Blynk\examples\Blynk.Edgent\Edgent_ESP32\BlynkEdgent.h"

void setup()
{
  Serial.begin(115200);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  delay(100);
  BlynkEdgent.begin();
}

BLYNK_WRITE(V0)
{
  int pinValue = param.asInt();
  digitalWrite(RED, pinValue);
}

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt();
  digitalWrite(GREEN, pinValue);
}

void loop() {
  BlynkEdgent.run();
}
