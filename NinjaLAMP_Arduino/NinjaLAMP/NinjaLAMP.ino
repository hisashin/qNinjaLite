#include <EEPROM.h>

struct ThermistorRange {
  double tempLowerLimit;
  int bConst;
  double voltageLimit;
};

#define USE_WIFI /* Use WiFi functionalities */
#define PIN_WIFI_MODE 16 /* Digitai input pin to switch AP (pairing) mode and Normal mode  */
#define VALUE_WIFI_MODE_AP HIGH /* Start with AP (pairing) mode if value of PIN_WIFI_MODE agrees with this.  */
bool isApMode = false;
void setup() {
  Serial.begin(9600);
  Serial.println("NinjaLAMP");
  EEPROM.begin(1024);
  pinMode(PIN_WIFI_MODE, INPUT);
  delay(100);
  isApMode = (digitalRead(PIN_WIFI_MODE)==VALUE_WIFI_MODE_AP);
  if (isApMode) {
    Serial.println("AP mode");
    startWiFiAPMode();
    
  } else {
    Serial.println("Core mode");
    startWiFiHTTPServerMode();
    setupCore();
  }
}

void loop() {
  /*
  if (isWiFiConnected()) {
      loopWiFiHTTPServer();
  }
  */
  loopWiFiHTTPServer();
  /*
  if (isApMode) {
    
  } else {
    loopCore();
  }
  */
}
