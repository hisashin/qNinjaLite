#include "lcdgfx.h"
#include "lcdgfx_gui.h"
#include "NinjaLAMPDefs.h"
#include "NinjaLAMPTopMenu.h"

extern DisplaySSD1306_128x64_I2C display;

/* Initialize screen buttons */
LcdGfxButton btn_run( "RUN a Cycle", {{10,5},{118,30}} );
LcdGfxButton btn_setup( "Setup Cycles", {{10,35},{118,60}} );

NinjaLAMPTopMenu::NinjaLAMPTopMenu () {

}

void NinjaLAMPTopMenu::setup (int moveUp, int ok, int moveDown) {
  pinMoveUp = moveUp;
  pinOk = ok;
  pinMoveDown = moveDown;
  display.clear();
  btn_run.setFocus(true);
  btn_run.show(display);
  btn_setup.setFocus(false);
  btn_setup.show(display);
}

boolean moveUpLow, okLow, moveDownLow;

int NinjaLAMPTopMenu::loop(int state){

  if(digitalRead(pinMoveUp) == HIGH) {
    if(moveUpLow){
      moveUpLow = false;
      swapBtnFocus();
    }
  } else {
    moveUpLow = true;
  }

  if(digitalRead(pinOk) == HIGH) {
    if(okLow){
      okLow = false;
      state = btn_run.isActive() ? STATE_RUN_MENU : STATE_SETUP_MENU;
    }
  } else {
    okLow = true;
  }

  if(digitalRead(pinMoveDown) == HIGH) {
    if(moveDownLow){
      moveDownLow = false;
      swapBtnFocus();
    }
  } else {
    moveDownLow = true;
  }
  return state;
}

void NinjaLAMPTopMenu::swapBtnFocus(){

  if(btn_run.isActive()){
    btn_run.setFocus(false);
    btn_run.show(display);
    btn_setup.setFocus(true);
    btn_setup.show(display);
  }else{
    btn_run.setFocus(true);
    btn_run.show(display);
    btn_setup.setFocus(false);
    btn_setup.show(display);
  }
}
