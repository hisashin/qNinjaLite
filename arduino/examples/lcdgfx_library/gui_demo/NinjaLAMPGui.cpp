#include "lcdgfx.h"
#include "NinjaLAMPGui.h"
#include "NinjaLAMPSysConfig.h"
#include "NinjaLAMPStates.h"
#include "NinjaLAMPTopMenu.h"
#include "NinjaLAMPMenu.h"

#define DEBUG_GUI 1

// Components
NinjaLAMPSysConfig sysConfig;
NinjaLAMPTopMenu topMenu;
NinjaLAMPMenu menu;

// Constructor
NinjaLAMPGui::NinjaLAMPGui () {

}

void NinjaLAMPGui::setup () {
  #ifdef DEBUG
    Serial.println("NinjaLAMPGui::setup");
  #endif
  // Set up and read the configuration.
  sysConfig.setup();
  #ifdef DEBUG_GUI
    Serial.print("NinjaLAMPGui::setup orientation_left_handed: ");
    Serial.println(sysConfig.orientation_left_handed);
    Serial.print("NinjaLAMPGui::setup cycleMaxCount: ");
    Serial.println(sysConfig.cycleMaxCount);
    Serial.print("NinjaLAMPGui::setup cycleCount: ");
    Serial.println(sysConfig.cycleCount); 
    //
    for(int i = 0; i < sysConfig.cycleCount; i++){
      Serial.print("NinjaLAMPGui::setup cycle ");
      Serial.println(i);      
      Serial.print("NinjaLAMPGui::setup amplifyTemp: ");
      Serial.println(sysConfig.cycles[i].amplifyTemp);
      Serial.print("NinjaLAMPGui::setup amplifyTime: ");
      Serial.println(sysConfig.cycles[i].amplifyTime);
      Serial.print("NinjaLAMPGui::setup stopTemp: ");
      Serial.println(sysConfig.cycles[i].stopTemp);
      Serial.print("NinjaLAMPGui::setup stopTime: ");
      Serial.println(sysConfig.cycles[i].stopTime); 
    }
  #endif
  // Change value of pins if orientation is for left handed, get orientation from EEPROM
  if(sysConfig.orientation_left_handed){
    pinMoveUp = 8;     // move up
    pinOk = 9;         // enter
    pinMoveDown = 10;  // move down   
  }
  pinMode(pinMoveUp, INPUT);
  pinMode(pinOk, INPUT);
  pinMode(pinMoveDown, INPUT);
  
  topMenu.setup(pinMoveUp, pinOk, pinMoveDown, sysConfig.orientation_left_handed);
  menu.setup(pinMoveUp, pinOk, pinMoveDown, sysConfig.orientation_left_handed);
}

int NinjaLAMPGui::loop (int state) {
  switch(state){
    case STATE_TOP_MENU:
      old_state = state;
      state = topMenu.loop(state);
      break;
    case STATE_RUN_MENU:
      if(state != old_state){
        #ifdef DEBUG_GUI
          Serial.print("NinjaLAMPGui::loop - Current state: STATE_RUN_MENU (");
          Serial.print(state);
          Serial.print(") - old_state: ");
          Serial.println(old_state);
        #endif
        initRunMenu();
      }
      old_state = state;
      selectedMenuItem = menu.loop();
      if(selectedMenuItem >= 0) {
        #ifdef DEBUG_GUI
          Serial.print("NinjaLAMPGui::loop - selectedMenuItem: ");
          Serial.println(selectedMenuItem);
        #endif
        switch(selectedMenuItem){
          case 0:
            state = STATE_TOP_MENU;
            topMenu.setup(pinMoveUp, pinOk, pinMoveDown, sysConfig.orientation_left_handed);
            break;
          default:
            selectedMenuItem = -1;
        }
      }
      break;
    case STATE_SETUP_MENU:
      if(state != old_state){
        #ifdef DEBUG_GUI
          Serial.print("NinjaLAMPGui::loop - Current state: STATE_SETUP_MENU (");
          Serial.print(state);
          Serial.print(") - old_state: ");
          Serial.println(old_state);
        #endif
        initSetupMenu();
      }
      old_state = state;
      selectedMenuItem = menu.loop();
      if(selectedMenuItem >= 0) {
        #ifdef DEBUG_GUI
          Serial.print("NinjaLAMPGui::loop - selectedMenuItem: ");
          Serial.println(selectedMenuItem);
        #endif
        switch(selectedMenuItem){
          case 0:
            state = STATE_TOP_MENU;
            topMenu.setup(pinMoveUp, pinOk, pinMoveDown, sysConfig.orientation_left_handed);
            break;
          default:
            selectedMenuItem = -1;
        }
      }
      break;
  }
  delay(100);
  return state;
}

void NinjaLAMPGui::initSetupMenu() {
  const char *menuItems[sysConfig.cycleCount + 3];
  menuItems[0] = "Back";
  for(int i = 0; i < sysConfig.cycleCount; i++){
    menuItems[i + 1] = sysConfig.cycles[i].menuItem;
  }
  if(sysConfig.cycleCount + 1 < MAX_MENU_ITEMS){
    menuItems[sysConfig.cycleCount + 1] = "Add a cycle";
    menuItems[sysConfig.cycleCount + 2] = "Flip orient'n";
  }else{
    menuItems[sysConfig.cycleCount + 1] = "Flip orient'n";
  }

  #ifdef DEBUG_GUI
    Serial.print("NinjaLAMPGui::initSetupMenu - menuItems len: ");
    Serial.print(sizeof(menuItems) / sizeof(char *));
    Serial.print(" - item 2: ");
    Serial.println(menuItems[2]);
    Serial.print("NinjaLAMPGui::initSetupMenu - sizeof(char *): ");
    Serial.println(sizeof(char *));
    Serial.print("NinjaLAMPGui::initSetupMenu - sizeof(menuItems): ");
    Serial.println(sizeof(menuItems));
  #endif
  menu.setMenu( menuItems, sizeof(menuItems) / sizeof(char *) );
}

void NinjaLAMPGui::initRunMenu() {
  const char *menuItems[sysConfig.cycleCount + 1];
  menuItems[0] = "Back";
  for(int i = 0; i < sysConfig.cycleCount; i++){
    menuItems[i + 1] = sysConfig.cycles[i].menuItem;
  }

  #ifdef DEBUG_GUI
    Serial.print("NinjaLAMPGui::initRunMenu - menuItems len: ");
    Serial.print(sizeof(menuItems) / sizeof(char *));
    Serial.print(" - item 2: ");
    Serial.println(menuItems[2]);
    Serial.print("NinjaLAMPGui::initRunMenu - sizeof(char *): ");
    Serial.println(sizeof(char *));
    Serial.print("NinjaLAMPGui::initRunMenu - sizeof(menuItems): ");
    Serial.println(sizeof(menuItems));
  #endif
  menu.setMenu( menuItems, sizeof(menuItems) / sizeof(char *) );
}
