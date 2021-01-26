#include "NinjaLAMPGui.h"
#include "NinjaLAMPDefs.h"

#include "ADCArduino.h"
#include <NinjaLAMPCore.h>
#include "HardwareConf.h"

#define DEBUG 1

NinjaLAMPGui gui;
NinjaLAMPCore *core = HardwareConf::initLCDModel();

// Values defined in NinjaLAMPStates.h
int state = STATE_TOP_MENU;

void setup()
{
  #ifdef DEBUG
  Serial.begin(9600);
  delay(2000);  // this is needed to assure that the serial communication is established.
  Serial.println("gui_demo setup");
  #endif
  gui.setCore(core);
  gui.setup();
}

void loop()
{
  // ToDo: need to detect a change in state
  switch(state){
    case STATE_TOP_MENU:
    case STATE_RUN_MENU:
    case STATE_SETUP_MENU:
      state = gui.loop(state);
      break;
    case STATE_SETUP_CYCLE:
      state = gui.loop(state);
      break;
    case STATE_RUN_CYCLE:
      state = gui.loop(state);
      break;
    default:
      break;
  }
}
