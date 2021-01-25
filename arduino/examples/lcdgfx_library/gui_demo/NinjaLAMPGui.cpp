#include "lcdgfx.h"
#include "NinjaLAMPGui.h"
#include "NinjaLAMPSysConfig.h"
#include "NinjaLAMPDefs.h"
#include "NinjaLAMPTopMenu.h"
#include "NinjaLAMPMenu.h"
#include "NinjaLAMPSetupCyclePage.h"
#include "NinjaLAMPRunCyclePage.h"

#define DEBUG_GUI 1
#define RESET_EEPROM 1

// Components
NinjaLAMPSysConfig sysConfig;
NinjaLAMPTopMenu topMenu;
NinjaLAMPMenu menu;
NinjaLAMPSetupCyclePage cyclePage;
NinjaLAMPRunCyclePage runPage;
DisplaySSD1306_128x64_I2C display(-1);

// Constructor
NinjaLAMPGui::NinjaLAMPGui () {

}

void NinjaLAMPGui::setup () {
  #ifdef DEBUG
    Serial.println("NinjaLAMPGui::setup");
  #endif
  // Initialize the display
  // NOTE: The display starts in right handed mode.  We should consider to quicklt read the 
  //       orientation from EEPROM and set it right away so we don't show the initial message 
  //       upside down. Could create sysConfig.get_orientation_left_handed() public function.
  display.begin();
  // In fixed text mode we have 4 rows of 16 characters.
  display.setFixedFont(ssd1306xled_font8x16);
  display.clear();
  display.printFixed(0,  8, "Reading configuration ...", STYLE_NORMAL);
  #ifdef RESET_EEPROM
  sysConfig.resetEEPROM();
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
    // Flip the screen 180 degrees
    display.getInterface().flipHorizontal(1);
    display.getInterface().flipVertical(1);
  }else{
    pinMoveUp = 10;   // move up
    pinOk = 9;        // enter
    pinMoveDown = 8;  // move down
  }
  pinMode(pinMoveUp, INPUT);
  pinMode(pinOk, INPUT);
  pinMode(pinMoveDown, INPUT);
  
  display.clear();    // This clear the "Configuration loading ..." message.
  
  topMenu.setup(pinMoveUp, pinOk, pinMoveDown);
  menu.setup(pinMoveUp, pinOk, pinMoveDown);
  cyclePage.setup(pinMoveUp, pinOk, pinMoveDown);
  
}

int NinjaLAMPGui::loop (int state) {
  switch(state){
    // ==============
    // STATE_TOP_MENU
    // ==============
    case STATE_TOP_MENU:
      old_state = state;
      state = topMenu.loop(state);
      break;
    // ==============
    // STATE_RUN_MENU
    // ==============
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
        // This logic needs to be coordinated with the setup menu content in NinjaLAMPGui::initRunMenu
        switch(selectedMenuItem){
          case 0:
            state = STATE_TOP_MENU;
            topMenu.setup(pinMoveUp, pinOk, pinMoveDown);
            break;
          default:
            state = STATE_RUN_CYCLE;
        }
      }
      break;
    // =================
    // STATE_RUN_CYCLE
    // =================
    case STATE_RUN_CYCLE:
      if(state != old_state){
        #ifdef DEBUG_GUI
          Serial.print("NinjaLAMPGui::loop - Current state: STATE_RUN_CYCLE (");
          Serial.print(state);
          Serial.print(") - old_state: ");
          Serial.println(old_state);
        #endif
        initRunPage(selectedMenuItem);
      }
      old_state = state;
      state = runPage.loop(state);
      break;
    // ================
    // STATE_SETUP_MENU
    // ================
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
        // This logic needs to be coordinated with the setup menu content in NinjaLAMPGui::initSetupMenu
        if(selectedMenuItem == 0){
          state = STATE_TOP_MENU;
          topMenu.setup(pinMoveUp, pinOk, pinMoveDown);
        }else if(selectedMenuItem <= sysConfig.cycleCount){
          state = STATE_SETUP_CYCLE;
        }else if(selectedMenuItem == sysConfig.cycleCount + 1){
          state = STATE_SETUP_CYCLE;
        }else if(selectedMenuItem == sysConfig.cycleCount + 2){
          state = STATE_TOP_MENU;
          sysConfig.flip_orientation();
          setup();
        }else{
          // should never get here
          selectedMenuItem = -1;
        }
      }
      break;
    // =================
    // STATE_SETUP_CYCLE
    // =================
    case STATE_SETUP_CYCLE:
      if(state != old_state){
        #ifdef DEBUG_GUI
          Serial.print("NinjaLAMPGui::loop - Current state: STATE_SETUP_CYCLE (");
          Serial.print(state);
          Serial.print(") - old_state: ");
          Serial.println(old_state);
        #endif
        initCyclePage(selectedMenuItem);
      }
      old_state = state;
      state = cyclePage.loop(state);
      break;
    // This should trigger an error?
    default:
      break;
  }
  
  delay(100);
  return state;
}

void NinjaLAMPGui::initSetupMenu() {
  // If you add more than 3 items revise MAX_MENU_CYCLES which drives sysConfig.cycleMaxCount
  const char *menuItems[sysConfig.cycleCount + 3];
  menuItems[0] = "Back";
  for(int i = 0; i < sysConfig.cycleCount; i++){
    menuItems[i + 1] = sysConfig.cycles[i].menuItem;
  }
  if(sysConfig.cycleCount + 1 < MAX_MENU_ITEMS){
    menuItems[sysConfig.cycleCount + 1] = "Add a cycle";
    menuItems[sysConfig.cycleCount + 2] = "Flip orient'on";
  }else{
    menuItems[sysConfig.cycleCount + 1] = "Flip orient'on";
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
  // If you add more than 3 items revise MAX_MENU_CYCLES which drives sysConfig.cycleMaxCount
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

void NinjaLAMPGui::initCyclePage(int selectedMenuItem){
  int cyclesIdx = -1;
  // This logic needs to be coordinated with the setup menu content in NinjaLAMPGui::initSetupMenu
  if(selectedMenuItem > 0 && selectedMenuItem <= sysConfig.cycleCount){
    cyclesIdx = selectedMenuItem - 1;
    #ifdef DEBUG_GUI
      char buff[80];
      sprintf(buff, "initCyclePage - selected %s", sysConfig.cycles[cyclesIdx].menuItem); 
      Serial.println(buff);
    #endif
  }
  cyclePage.initPage(cyclesIdx);
}

void NinjaLAMPGui::initRunPage(int selectedMenuItem){
  int cyclesIdx = -1;
  if(selectedMenuItem > 0 && selectedMenuItem <= sysConfig.cycleCount){
    cyclesIdx = selectedMenuItem - 1;
    #ifdef DEBUG_GUI
      char buff[80];
      sprintf(buff, "initRunPage - selected %s", sysConfig.cycles[cyclesIdx].menuItem); 
      Serial.println(buff);
    #endif
  }
  runPage.initPage(cyclesIdx);
}  
