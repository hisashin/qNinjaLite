#include "lcdgfx.h"
#include "lcdgfx_gui.h"
#include "NinjaLAMPRunCyclePage.h"
#include "NinjaLAMPSysConfig.h"
#include "NinjaLAMPDefs.h"

#define DEBUG_RUN_CYCLE_PAGE 1

extern DisplaySSD1306_128x64_I2C display;
extern NinjaLAMPSysConfig sysConfig;

// Constructor
NinjaLAMPRunCyclePage::NinjaLAMPRunCyclePage () {

}

void NinjaLAMPRunCyclePage::setup (int moveUp, int ok, int moveDown) {
  #ifdef DEBUG
    Serial.println("NinjaLAMPRunCyclePage::setup");
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

int NinjaLAMPRunCyclePage::loop(int state) {
  
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
        // TBD
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
          case SEL_RUNNING:

            break;
          case SEL_CANCEL:
            state = STATE_RUN_MENU;
            break;
          default:

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
        // TBD
      }    
    }
  } else {
    pinMoveDownLow = true;
  }

  drawPage();
  return state;
}

void NinjaLAMPRunCyclePage::initPage(int _cyclesIdx){
  #ifdef DEBUG_RUN_CYCLE_PAGE
    Serial.print("NinjaLAMPSetupCyclePage::initPage _cyclesIdx: ");
    Serial.println(_cyclesIdx);
  #endif
  cyclesIdx = _cyclesIdx;
  // Check for valid cyclesIdx, it could be -1
  
  // TODO: start the cycle


  display.clear();
  selection = SEL_RUNNING;
  mode = MODE_NAVIGATION;
  // drawPage will be called in the loop.
}
void NinjaLAMPRunCyclePage::drawPage (){
  // Run control.
  if(selection == SEL_RUNNING && mode == MODE_NAVIGATION) display.invertColors();
  display.printFixed(0,  0, "Running", STYLE_NORMAL);
  if(selection == SEL_RUNNING && mode == MODE_NAVIGATION) display.invertColors();

  if(selection == SEL_CANCEL && mode == MODE_NAVIGATION) display.invertColors();
  display.printFixed(72,  0, "Cancel", STYLE_NORMAL);
  if(selection == SEL_CANCEL && mode == MODE_NAVIGATION) display.invertColors();
}
