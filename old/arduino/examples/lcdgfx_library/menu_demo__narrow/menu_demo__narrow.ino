/*
    MIT License

    Copyright (c) 2017-2019, Alexey Dynda

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
/*
 *   Attiny85 PINS
 *             ____
 *   RESET   -|_|  |- 3V
 *   SCL (3) -|    |- (2)
 *   SDA (4) -|    |- (1)
 *   GND     -|____|- (0) - BUTTONS module
 *
 *   Atmega328 PINS: connect LCD to A4/A5,
 *   Z-keypad ADC module on A0 pin.
 */

#include "lcdgfx.h"
#include "lcdgfx_gui.h"

#define menuUp 10   /* move up */
#define menuEnter 9 /* enter */
#define menuDown 8  /* move down */

DisplaySSD1306_128x64_I2C display(-1);

/* Define menu items of the menu box */
const char *menuItems[] =
{
  "menu item 0",
  "menu item 1",
  "menu item 2",
  "menu item 3",
  "menu item 4",
  "menu item 5",
  "menu item 6",
};

// Leave 28 pixels on the right.
LcdGfxMenu menu( menuItems, sizeof(menuItems) / sizeof(char *), (NanoRect){{0,0},{100,0} } );

void setup()
{
  pinMode(menuUp, INPUT);
  pinMode(menuEnter, INPUT);
  pinMode(menuDown, INPUT);

  Serial.begin(9600);

  display.begin();
  display.setFixedFont(ssd1306xled_font8x16);
  display.clear();

  menu.show(display);
}

boolean menuUpLow, menuEnterLow, menuDownLow;
int selectedMenuItem;

void loop()
{
  if(digitalRead(menuUp) == HIGH) {
    if(menuUpLow){
      menuUpLow = false;
      /* Move the menu cursor up and refresh the menu on the display */
      menu.up();
      menu.show( display );    
    }
  } else {
    menuUpLow = true;
  }

  if(digitalRead(menuEnter) == HIGH) {
    if(menuEnterLow){
      menuEnterLow = false;
      selectedMenuItem = menu.selection( );
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
      menu.down();
      menu.show( display );     
    }
  } else {
    menuDownLow = true;
  }

  delay(100);
}
