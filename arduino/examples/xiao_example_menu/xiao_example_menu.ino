/*
 * Make sure you have the ssd1306 in the sketch include libraries.
 * 
 * This example creates a menu with 10 items and uses the TACT0, TACT1 and TACT2 switches 
 * to move the menu selection and select a menu item.
 * 
 * To run the example 
 * 1. load the file arduino.examples/xiao_example_menu/xiao_example_menu.ino nto the Arduino IDE
 * 2, click the magnifying glass icon at the top-right corner and set rate to "9600 baud"
 * 3. Upload the code
 * 4. tap the switcjes to change the menu selection. The serial monitor will show the selection when you
 *    tap the middle switch (enter) - menuEnter - selected menu item X
 */
#include "ssd1306.h"

#define menuUp 10   /* move up */
#define menuEnter 9 /* enter */
#define menuDown 8  /* move down */

/* Define menu items */
const char *menuItems[] =
{
    "menu item 0",  
    "menu item 1",
    "menu item 2",
    "menu item 3",
    "menu item 4",
    "menu item 5",
    "menu item 6",
    "menu item 7",
    "menu item 8",
    "menu item 9",
};
/* This variable will hold menu state; it is processed by SSD1306 API functions */
SAppMenu menu;

void setup(){
  pinMode(menuUp, INPUT);
  pinMode(menuEnter, INPUT);
  pinMode(menuDown, INPUT);
  Serial.begin(9600);

  ssd1306_128x64_i2c_init();
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_fillScreen( 0x00 );
  /* Initialize the main menu state */
  ssd1306_createMenu( &menu, menuItems, sizeof(menuItems) / sizeof(char *) );
  /* Show the menu on the display */
  ssd1306_showMenu( &menu );
}

boolean menuUpLow, menuEnterLow, menuDownLow;
int selectedMenuItem;

void loop()
{
  if(digitalRead(menuUp) == HIGH) {
    if(menuUpLow){
      menuUpLow = false;
      /* Move the menu cursor up and refresh the menu on the display */
      ssd1306_menuUp( &menu );
      ssd1306_updateMenu( &menu );      
    }
  } else {
    menuUpLow = true;
  }

  if(digitalRead(menuEnter) == HIGH) {
    if(menuEnterLow){
      menuEnterLow = false;
      selectedMenuItem = ssd1306_menuSelection( &menu );
      Serial.print("menuEnter - selected menu item ");
      Serial.print(selectedMenuItem);
      Serial.println();    
    }
  } else {
    menuEnterLow = true;
  }


  if(digitalRead(menuDown) == HIGH) {
    if(menuDownLow){
      menuDownLow = false;
      /* Move the menu cursor down and refresh the menu on the display */
      ssd1306_menuDown( &menu );
      ssd1306_updateMenu( &menu );      
    }
  } else {
    menuDownLow = true;
  }

  delay(100);
}
