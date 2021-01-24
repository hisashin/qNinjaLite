#include "lcdgfx.h"

class NinjaLAMPTopMenu {
  public:
    NinjaLAMPTopMenu();
    void setup(int pinMoveUp, int pinOk, int pinMoveDown); 
    int loop(int state);
  private:
    // The pins will be set in setup depending on the orientation.
    int pinMoveUp = 0;   // move up
    int pinOk = 0;        // enter
    int pinMoveDown = 0;  // move down
    void swapBtnFocus();
};
