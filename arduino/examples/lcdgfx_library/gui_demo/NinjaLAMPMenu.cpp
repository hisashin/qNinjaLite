#include "NinjaLAMPMenu.h"
#include "lcdgfx.h"
#include "lcdgfx_gui.h"

#define menuUp 10   /* move up */
#define menuEnter 9 /* enter */
#define menuDown 8  /* move down */

#define DEBUG_MENU 1

DisplaySSD1306_128x64_I2C menu_display(-1);

// Constructor
NinjaLAMPMenu::NinjaLAMPMenu () {

}

void NinjaLAMPMenu::setup (int moveUp, int ok, int moveDown, boolean left_handed) {
  #ifdef DEBUG
    Serial.println("NinjaLAMPMenu::setup");
  #endif
  pinMoveUp = moveUp;
  pinOk = ok;
  pinMoveDown = moveDown;


}

int NinjaLAMPMenu::loop() {
  if(digitalRead(menuUp) == HIGH) {
    if(menuUpLow){
      menuUpLow = false;
      /* Move the menu cursor up and refresh the menu on the display */
      menu->up();
      menu->show( menu_display );    
    }
  } else {
    menuUpLow = true;
  }

  if(digitalRead(menuEnter) == HIGH) {
    if(menuEnterLow){
      menuEnterLow = false;
      selectedMenuItem = menu->selection( );
      Serial.print("menuEnter - selected menu item ");
      Serial.print(selectedMenuItem);
      Serial.println();  
      return selectedMenuItem;
    }
  } else {
    menuEnterLow = true;
  }


  if(digitalRead(menuDown) == HIGH) {
    if(menuDownLow){
      menuDownLow = false;
      /* Move the menu cursor down and refresh the menu on the display */
      menu->down();
      menu->show( menu_display );     
    }
  } else {
    menuDownLow = true;
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
  
  menu_display.begin();
  menu_display.setFixedFont(ssd1306xled_font8x16);
  menu_display.clear();

  menu->show(menu_display);
}
 
