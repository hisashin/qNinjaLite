#ifndef __NINJA_LAMP_SETUP_CYCLE_PAGE__
#define __NINJA_LAMP_SETUP_CYCLE_PAGE__
#include "Arduino.h"

#define SEL_AMP_C 1
#define SEL_AMP_M 2
#define SEL_STOP_C 3
#define SEL_STOP_M 4
#define SEL_OK 5
#define SEL_CANCEL 6
#define SEL_MAX 6
#define MODE_NAVIGATION 1
#define MODE_SELECTION 2

class NinjaLAMPSetupCyclePage {
  public:
    NinjaLAMPSetupCyclePage();
    void setup(int moveUp, int ok, int moveDown);
    int loop(int state);  
    void initPage(int cyclesIdx);
  private:
    int cyclesIdx;
    int selection;
    int mode;
    // Side switches, pin number depends on orientation written in EEPROM, reset in setup
    int pinMoveUp = 10;   // move up
    int pinOk = 9;        // enter
    int pinMoveDown = 8;  // move down
    boolean pinMoveUpLow, pinOkLow, pinMoveDownLow;
    void drawPage();
    void increaseValue();
    void decreaseValue();
    void saveCycle();
    void addCycle();
};
#endif /* __NINJA_LAMP_SETUP_CYCLE_PAGE__ */