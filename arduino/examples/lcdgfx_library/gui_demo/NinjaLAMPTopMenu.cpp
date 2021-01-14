#include "lcdgfx.h"
#include "lcdgfx_gui.h"
#include "NinjaLAMPStates.h"
#include "NinjaLAMPTopMenu.h"

DisplaySSD1306_128x64_I2C tm_display(-1);

/* Initialize screen buttons */
LcdGfxButton btn_run( "RUN a Cycle", {{10,5},{118,30}} );
LcdGfxButton btn_setup( "Setup Cycles", {{10,35},{118,60}} );

NinjaLAMPTopMenu::NinjaLAMPTopMenu () {

}

void NinjaLAMPTopMenu::setup (int moveUp, int ok, int moveDown, boolean left_handed) {
  pinMoveUp = moveUp;
  pinOk = ok;
  pinMoveDown = moveDown;

  tm_display.begin();
  tm_display.setFixedFont(ssd1306xled_font8x16);
  tm_display.clear();

  if(left_handed){   // Flip 180 degrees
    tm_display.getInterface().flipHorizontal(1);
    tm_display.getInterface().flipVertical(1);
  }
  
  btn_run.setFocus(true);
  btn_run.show(tm_display);
  btn_setup.setFocus(false);
  btn_setup.show(tm_display);
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
    btn_run.show(tm_display);
    btn_setup.setFocus(true);
    btn_setup.show(tm_display);
  }else{
    btn_run.setFocus(true);
    btn_run.show(tm_display);
    btn_setup.setFocus(false);
    btn_setup.show(tm_display);
  }
}
