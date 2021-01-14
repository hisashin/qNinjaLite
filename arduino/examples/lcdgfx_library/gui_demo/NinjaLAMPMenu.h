#include "Arduino.h"
#include "lcdgfx.h"
#include "lcdgfx_gui.h"

#define MAX_MENU_ITEMS 40

class NinjaLAMPMenu {
  public:
    NinjaLAMPMenu();
    void setup(int moveUp, int ok, int moveDown, boolean left_handed);
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
    boolean menuUpLow, menuEnterLow, menuDownLow;
    int selectedMenuItem;

};
