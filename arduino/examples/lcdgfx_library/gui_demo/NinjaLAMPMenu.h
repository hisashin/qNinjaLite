#ifndef __NINJA_LAMP_MENU__
#define __NINJA_LAMP_MENU__
#include "Arduino.h"
#include "lcdgfx.h"
#include "lcdgfx_gui.h"
#include "NinjaLAMPDefs.h"

class NinjaLAMPMenu {
  public:
    NinjaLAMPMenu();
    void setup(int moveUp, int ok, int moveDown);
    int loop(); 
    void setMenu( const char **  _menuItems, uint8_t menuItemsSize );

  private:
    LcdGfxMenu *menu;
    const char *menuItems[MAX_MENU_ITEMS];
    // The pins will be set in setup depending on the orientation.
    int pinMoveUp = 0;    // move up
    int pinOk = 0;        // enter
    int pinMoveDown = 0;  // move down
    //
    boolean pinMoveUpLow, pinOkLow, pinMoveDownLow;
    int selectedMenuItem;

};
#endif /* __NINJA_LAMP_MENU__ */