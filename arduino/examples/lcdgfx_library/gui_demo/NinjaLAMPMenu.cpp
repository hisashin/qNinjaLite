#include "NinjaLAMPMenu.h"
#include "lcdgfx.h"
#include "lcdgfx_gui.h"

#define DEBUG_MENU 1

extern DisplaySSD1306_128x64_I2C display;

// Constructor
NinjaLAMPMenu::NinjaLAMPMenu () {

}

void NinjaLAMPMenu::setup (int moveUp, int ok, int moveDown) {
  #ifdef DEBUG
    Serial.println("NinjaLAMPMenu::setup");
  #endif
  pinMoveUp = moveUp;
  pinOk = ok;
  pinMoveDown = moveDown;
}

int NinjaLAMPMenu::loop() {
  
  if(digitalRead(pinMoveUp) == HIGH) {
    if(pinMoveUpLow){
      pinMoveUpLow = false;
      /* Move the menu cursor up and refresh the menu on the display */
      menu->up();
      menu->show( display );    
    }
  } else {
    pinMoveUpLow = true;
  }

  if(digitalRead(pinOk) == HIGH) {
    if(pinOkLow){
      pinOkLow = false;
      selectedMenuItem = menu->selection( );
      Serial.print("pinOk - selected menu item ");
      Serial.print(selectedMenuItem);
      Serial.println();  
      return selectedMenuItem;
    }
  } else {
    pinOkLow = true;
  }

  if(digitalRead(pinMoveDown) == HIGH) {
    if(pinMoveDownLow){
      pinMoveDownLow = false;
      /* Move the menu cursor down and refresh the menu on the display */
      menu->down();
      menu->show( display );     
    }
  } else {
    pinMoveDownLow = true;
  }
  
  return -1;
}

void NinjaLAMPMenu::setMenu( const char **  _menuItems, uint8_t menuItemsSize ) {
  #ifdef DEBUG_MENU
    Serial.print("NinjaLAMPMenu::setMenu - _menuItems elements");
    Serial.print(menuItemsSize );
    Serial.print(" - item 2 lwn: ");
    Serial.println(sizeof(_menuItems[2]));
  #endif

  for(int i = 0; i < menuItemsSize; i++) {
    menuItems[i] = _menuItems[i];
  }

  #ifdef DEBUG_MENU
    Serial.print("NinjaLAMPMenu::setMenu - menuItems elements");
    Serial.print(sizeof(menuItems) / sizeof(char *)  );
    Serial.print(" - item 2: ");
    Serial.println(_menuItems[2]);
  #endif

  if( menu){
    delete menu; 
    #ifdef DEBUG_MENU
      Serial.println("NinjaLAMPMenu::setMenu - deleted menu object");
    #endif
  }
  menu = new LcdGfxMenu(menuItems, menuItemsSize); // create new menu
  display.clear();
  menu->show(display);
}
 
