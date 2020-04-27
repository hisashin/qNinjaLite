#ifndef _NINJALAMP_CORE_H_
#define _NINJALAMP_CORE_H_

#include "Arduino.h"

#define TARGET_TEMP 63

struct ThermistorRange {
  double tempLowerLimit;
  int bConst;
  double voltageLimit;
};

class NinjaLAMPCore {
  public:
    NinjaLAMPCore();
    // Called from Arduino's setup & loop functions
    void setup();
    void loop();
    // Called by interfaces
    void start(double temp);
    void stop();
  private:
    bool started;
};

#endif _NINJALAMP_CORE_H_
