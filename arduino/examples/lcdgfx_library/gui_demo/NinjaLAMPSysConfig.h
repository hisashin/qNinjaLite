#include "Arduino.h"



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
