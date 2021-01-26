#ifndef __NINJA_LAMP_SYS_CONFIG__
#define __NINJA_LAMP_SYS_CONFIG__

#include "Arduino.h"

struct Cycle {
  int amplifyTemp; // Celsius
  int amplifyTime; // Sec
  int stopTemp; // Celsius
  int stopTime; // Sec
  char menuItem[15];
};

class NinjaLAMPSysConfig {
  public:
    NinjaLAMPSysConfig();
    void resetEEPROM();
    void setup();
    byte orientation_left_handed;
    int cycleMaxCount;
    Cycle* cycles = 0;
    int cycleCount;
    void save_config();
    void add_cycle(Cycle new_cycle);
    void flip_orientation();
  private:
    void set_cycles();
};
#endif /* __NINJA_LAMP_SYS_CONFIG__ */