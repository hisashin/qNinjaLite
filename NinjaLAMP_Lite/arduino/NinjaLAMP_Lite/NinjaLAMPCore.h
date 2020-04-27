#ifndef _NINJALAMP_CORE_H_
#define _NINJALAMP_CORE_H_

#include "Arduino.h"


struct ThermistorRange {
  double tempLowerLimit;
  int bConst;
  double voltageLimit;
};

class NinjaLAMPCore {
  public:
    NinjaLAMPCore();
    void setup();
    void loop();
  private:
};

#endif _NINJALAMP_CORE_H_
