#include "Arduino.h"

#define SEL_RUNNING 1
#define SEL_CANCEL 2
#define SEL_MAX 2
#define MODE_NAVIGATION 1
#define MODE_SELECTION 2

class NinjaLAMPRunCyclePage {
  public:
    NinjaLAMPRunCyclePage();
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

};
