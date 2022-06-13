#ifndef __NINJA_LAMP_RUN_CYCLE_PAGE__
#define __NINJA_LAMP_RUN_CYCLE_PAGE__

#include "Arduino.h"
#include <NinjaLAMPCore.h>
#include "NinjaLAMPSysConfig.h"

#define SEL_RUNNING 1
#define SEL_CANCEL 2
#define SEL_MAX 2
#define MODE_NAVIGATION 1
#define MODE_SELECTION 2


enum ExperimentPhase {
  none,
  amplify,
  stop,
  complete
};

class NinjaLAMPRunCyclePage {
  public:
    NinjaLAMPRunCyclePage();
    void setup(int moveUp, int ok, int moveDown);
    int loop(int state);
    void initPage(int cyclesIdx, NinjaLAMPCore *core);
  private:
    int cyclesIdx;
    int selection;
    int mode;
    ExperimentPhase phase = none;
    
    NinjaLAMPCore *core;
    int loopUI(int state);
    void loopThermalCycler();
    // Side switches, pin number depends on orientation written in EEPROM, reset in setup
    int pinMoveUp = 10;   // move up
    int pinOk = 9;        // enter
    int pinMoveDown = 8;  // move down
    boolean pinMoveUpLow, pinOkLow, pinMoveDownLow;
    void drawPage();
    boolean isRunning ();

};
#endif /* __NINJA_LAMP_RUN_CYCLE_PAGE__ */
