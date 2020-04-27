#include <EEPROM.h>

struct ThermistorRange {
  double tempLowerLimit;
  int bConst;
  double voltageLimit;
};

bool isApMode = false;
void setup() {
  Serial.begin(9600);
  Serial.println("NinjaLAMP");
  setupCore();
}

void loop() {
  loopCore();
}
