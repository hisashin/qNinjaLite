/*
    MIT License

    Copyright (c) 2020, Alexey Dynda

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

#define menuUp 10   /* move up or left*/
#define menuEnter 9 /* enter */
#define menuDown 8  /* move down or right*/

DisplaySSD1306_128x64_I2C display(-1);

/* This variable will hold menu state, processed by SSD1306 API functions */
LcdGfxYesNo yesNoDialog( "Are you sure?" );

void setup()
{
  pinMode(menuUp, INPUT);
  pinMode(menuEnter, INPUT);
  pinMode(menuDown, INPUT);

  Serial.begin(9600);
  
  display.begin();
  display.setFixedFont(ssd1306xled_font8x16);
  display.clear();

  yesNoDialog.swapToNo();
  yesNoDialog.show(display);

}

boolean menuUpLow, menuEnterLow, menuDownLow;

void loop()
{
  if(digitalRead(menuUp) == HIGH) {
    if(menuUpLow){
      menuUpLow = false;
      yesNoDialog.swapToYes();
      yesNoDialog.show(display);  
    }
  } else {
    menuUpLow = true;
  }

  if(digitalRead(menuEnter) == HIGH) {
    if(menuEnterLow){
      menuEnterLow = false;
      Serial.print("ok pressed - selected button: ");
      Serial.print(yesNoDialog.isYes() ? "Yes" : "No");
      Serial.println();    
    }
  } else {
    menuEnterLow = true;
  }


  if(digitalRead(menuDown) == HIGH) {
    if(menuDownLow){
      menuDownLow = false;
      yesNoDialog.swapToNo();
      yesNoDialog.show(display);  
    }
  } else {
    menuDownLow = true;
  }

  delay(100);



  
//    Key newButton = getPressedButton(BUTTON_PIN);
//    if (newButton == btn)
//    {
//        return;
//    }
//    btn = newButton;
//    switch (btn)
//    {
//        case Key::BT_UP:
//        case Key::BT_LEFT:
//            /* move menu cursor up and refresh menu on the display */
//            yesNoDialog.swapToYes();
//            yesNoDialog.show(display);
//            break;
//        case Key::BT_DOWN:
//        case Key::BT_RIGHT:
//            /* move menu cursor down and refresh menu on the display */
//            yesNoDialog.swapToNo();
//            yesNoDialog.show(display);
//            break;
//        case Key::BT_SELECT:
//            /* You always can request current position of menu cursor, by calling display.menuSelection() */
//            break;
//        default:
//            break;
//    }
}
