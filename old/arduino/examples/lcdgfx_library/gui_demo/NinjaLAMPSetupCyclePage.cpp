#include "lcdgfx.h"
#include "lcdgfx_gui.h"
#include "NinjaLAMPSetupCyclePage.h"
#include "NinjaLAMPSysConfig.h"
#include "NinjaLAMPDefs.h"

#define DEBUG_SETUP_CYCLE_PAGE 1

extern DisplaySSD1306_128x64_I2C display;
extern NinjaLAMPSysConfig sysConfig;
Cycle cycle_buff;

// Constructor
NinjaLAMPSetupCyclePage::NinjaLAMPSetupCyclePage () {

}

void NinjaLAMPSetupCyclePage::setup (int moveUp, int ok, int moveDown) {
  #ifdef DEBUG
    Serial.println("NinjaLAMPSetupCyclePage::setup");
  #endif
  pinMoveUp = moveUp;
  pinOk = ok;
  pinMoveDown = moveDown;
}

/**
 * There are 2 modes: selection and navigation.
 * In navigation mode the up and down buttons move the selection from one item 
 *   to the other.
 * In selection mode the up and down buttons uncrease or decrease the value of 
 *   the item except of Ok and Cancel which will never enter selection mode. .
 * The Ok button toggle navigation / selection for all items except Ok which 
 *   will call saveCycle() and then returns the user to the dryup menu and 
 *   Cancel which returns the user to the setup menu without saving the changes.
 */

int NinjaLAMPSetupCyclePage::loop(int state) {
  
  if(digitalRead(pinMoveUp) == HIGH) {
    if(pinMoveUpLow){
      pinMoveUpLow = false;
      if(mode == MODE_NAVIGATION){
        /* Move the cursor to the previous itrm or wrap around */
        if(selection == 1){
          selection = SEL_MAX;
        }else{
          selection--;
        } 
      }else{
        increaseValue();
      }
    }
  } else {
    pinMoveUpLow = true;
  }

  if(digitalRead(pinOk) == HIGH) {
    if(pinOkLow){
      pinOkLow = false;
      if(mode == MODE_NAVIGATION){
        switch(selection){
          case SEL_OK:
            if(cyclesIdx >= 0){
              saveCycle();
            }else{
              addCycle();
            }
            state = STATE_SETUP_MENU;
            break;
          case SEL_CANCEL:
            state = STATE_SETUP_MENU;
            break;
          default:
            mode = MODE_SELECTION;
            break;
        }
      }else{
        mode = MODE_NAVIGATION;
      }
    }
  } else {
    pinOkLow = true;
  }

  if(digitalRead(pinMoveDown) == HIGH) {
    if(pinMoveDownLow){
      pinMoveDownLow = false;
      if(mode == MODE_NAVIGATION){
        /* Move the cursor to thr next itrm or wrap around */
        if(selection == SEL_MAX){
          selection = 1;
        }else{
          selection++;
        } 
      }else{
        decreaseValue();
      }    
    }
  } else {
    pinMoveDownLow = true;
  }

  drawPage();
  return state;
}

void NinjaLAMPSetupCyclePage::initPage(int _cyclesIdx){
  #ifdef DEBUG_SETUP_CYCLE_PAGE
    Serial.print("NinjaLAMPSetupCyclePage::initPage _cyclesIdx: ");
    Serial.println(_cyclesIdx);
  #endif
  cyclesIdx = _cyclesIdx;
  if(cyclesIdx >= 0){
    cycle_buff.amplifyTemp = sysConfig.cycles[cyclesIdx].amplifyTemp;
    cycle_buff.amplifyTime = sysConfig.cycles[cyclesIdx].amplifyTime;
    cycle_buff.stopTemp = sysConfig.cycles[cyclesIdx].stopTemp;
    cycle_buff.stopTime = sysConfig.cycles[cyclesIdx].stopTime;
  }else{
    cycle_buff.amplifyTemp = (byte) 60;
    cycle_buff.amplifyTime = (byte) 30;
    cycle_buff.stopTemp = (byte) 90;
    cycle_buff.stopTime = (byte) 10;
  }
  display.clear();
  selection = SEL_AMP_C;
  mode = MODE_NAVIGATION;
  // drawPage will be called in the loop.
}

void NinjaLAMPSetupCyclePage::drawPage(){
  // In ficed text mode we have 4 rows of 16 characters - set in NinjaLAMPGui::setup
  char buff[2];
  display.printFixed(20,  0, "Cycle Setup", STYLE_NORMAL);
  // Amplification
  display.printFixed(0,  16, "Amp", STYLE_NORMAL);
  if(selection == SEL_AMP_C && mode == MODE_NAVIGATION) display.invertColors();
  sprintf(buff, "%d", cycle_buff.amplifyTemp); 
  display.printFixed(32,  16, buff, STYLE_BOLD);
  if(selection == SEL_AMP_C && mode == MODE_NAVIGATION) display.invertColors();
  display.printFixed(48,  16, "c", STYLE_NORMAL);
  
  if(selection == SEL_AMP_M && mode == MODE_NAVIGATION) display.invertColors();
  sprintf(buff, "%d", cycle_buff.amplifyTime); 
  display.printFixed(64,  16, buff, STYLE_BOLD);
  if(selection == SEL_AMP_M && mode == MODE_NAVIGATION) display.invertColors();
  display.printFixed(80,  16, "min", STYLE_NORMAL); 
  // Stop
  display.printFixed(0,  32, "Stop", STYLE_NORMAL);
  if(selection == SEL_STOP_C && mode == MODE_NAVIGATION) display.invertColors();
  sprintf(buff, "%d", cycle_buff.stopTemp); 
  display.printFixed(40,  32, buff, STYLE_BOLD);
  if(selection == SEL_STOP_C && mode == MODE_NAVIGATION) display.invertColors();
  display.printFixed(56,  32, "c", STYLE_NORMAL);
  
  if(selection == SEL_STOP_M && mode == MODE_NAVIGATION) display.invertColors();
  sprintf(buff, "%d", cycle_buff.stopTime); 
  display.printFixed(72,  32, buff, STYLE_BOLD);
  if(selection == SEL_STOP_M && mode == MODE_NAVIGATION) display.invertColors();
  display.printFixed(88,  32, "min", STYLE_NORMAL); 
  // OK | Cancel
  if(selection == SEL_OK) display.invertColors();
  display.printFixed(16,  48, "Ok", STYLE_BOLD);
  if(selection == SEL_OK) display.invertColors();
  if(selection == SEL_CANCEL) display.invertColors();
  display.printFixed(56,  48, "Cancel", STYLE_BOLD);
  if(selection == SEL_CANCEL) display.invertColors();
}

void NinjaLAMPSetupCyclePage::increaseValue(){
  switch(selection){
    case SEL_AMP_C: cycle_buff.amplifyTemp++; break;
    case SEL_AMP_M: cycle_buff.amplifyTime++; break;
    case SEL_STOP_C: cycle_buff.stopTemp++; break;
    case SEL_STOP_M: cycle_buff.stopTime++; break;
  }
}

void NinjaLAMPSetupCyclePage::decreaseValue(){
  switch(selection){
    case SEL_AMP_C: cycle_buff.amplifyTemp--; break;
    case SEL_AMP_M: cycle_buff.amplifyTime--; break;
    case SEL_STOP_C: cycle_buff.stopTemp--; break;
    case SEL_STOP_M: cycle_buff.stopTime--; break;
  }
}

void NinjaLAMPSetupCyclePage::saveCycle(){
  #ifdef DEBUG_SETUP_CYCLE_PAGE
    Serial.println("NinjaLAMPSetupCyclePage::saveCycle");
  #endif
  sysConfig.cycles[cyclesIdx].amplifyTemp = cycle_buff.amplifyTemp;
  sysConfig.cycles[cyclesIdx].amplifyTime = cycle_buff.amplifyTime;
  sysConfig.cycles[cyclesIdx].stopTemp = cycle_buff.stopTemp;
  sysConfig.cycles[cyclesIdx].stopTime = cycle_buff.stopTime;
  sprintf(sysConfig.cycles[cyclesIdx].menuItem, "%dc%dm-%dc%dm", cycle_buff.amplifyTemp, cycle_buff.amplifyTime, cycle_buff.stopTemp, cycle_buff.stopTime); 
  sysConfig.save_config();
}

void NinjaLAMPSetupCyclePage::addCycle(){
  #ifdef DEBUG_SETUP_CYCLE_PAGE
    Serial.println("NinjaLAMPSetupCyclePage::addCycle");
  #endif
  display.clear();
  display.printFixed(0,  8, "Saving and reloading configuration ...", STYLE_NORMAL);
  sysConfig.add_cycle(cycle_buff);
}
