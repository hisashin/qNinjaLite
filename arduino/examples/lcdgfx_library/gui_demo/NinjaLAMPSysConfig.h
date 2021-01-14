#include "Arduino.h"

// Set data start from the end to allow the cycle array to grow.
// If we need to store more data we will limit the growth of the cycle array
// by setting EEPROM_CYCLES_MAX_COUNT appropriately
#define EEPROM_ORIENTATION_LEFT_HANDED 255
#define EEPROM_CYCLES_COUNT 254
#define EEPROM_CYCLES_MAX_COUNT 253
// The cycles array start from zero and grows
#define EEPROM_CYCLES_START 0
#define EEPROM_CYCLES_LEN 4

struct Cycle {
  int amplifyTemp;
  int amplifyTime;
  int stopTemp;
  int stopTime;
  char menuItem[15];
};

class NinjaLAMPSysConfig {
  public:
    NinjaLAMPSysConfig();
    void setup();
    byte orientation_left_handed;
    int cycleMaxCount;
    Cycle* cycles = 0;
    int cycleCount;
  private:
    void set_cycles();
};
