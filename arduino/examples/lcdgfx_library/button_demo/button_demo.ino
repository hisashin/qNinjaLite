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

DisplaySSD1306_128x64_I2C display(-1);

/* Initialize screen buttons */
LcdGfxButton btn_run( "RUN a Cycle", {{10,5},{118,30}} );
LcdGfxButton btn_setup( "Setup Cycles", {{10,35},{118,60}} );

// Side switches
#define moveUp 10   /* move up */
#define ok 9 /* enter */
#define moveDown 8  /* move down */

void setup()
{
  pinMode(moveUp, INPUT);
  pinMode(ok, INPUT);
  pinMode(moveDown, INPUT);

  Serial.begin(9600);
  
  display.begin();
  display.setFixedFont(ssd1306xled_font8x16);
  display.clear();

  btn_run.setFocus(true);
  btn_run.show(display);
  btn_setup.show(display);
  
}

boolean moveUpLow, okLow, moveDownLow;

// Wen the switch is pressed we set the Low flag so that when the switch is released 
// we run the proper code for that particular action.
void loop()
{
  if(digitalRead(moveUp) == HIGH) {
    if(moveUpLow){
      moveUpLow = false;
      swapBtnFocus();
    }
  } else {
    moveUpLow = true;
  }

  if(digitalRead(ok) == HIGH) {
    if(okLow){
      okLow = false;
      
      Serial.print("ok pressed - selected button: ");
      Serial.print(btn_run.isActive() ? "RUN" : "Setup");
      Serial.println(); 
 
    }
  } else {
    okLow = true;
  }

  if(digitalRead(moveDown) == HIGH) {
    if(moveDownLow){
      moveDownLow = false;
      swapBtnFocus();
    }
  } else {
    moveDownLow = true;
  }

  delay(100);
}

void swapBtnFocus(){

  if(btn_run.isActive()){
    btn_run.setFocus(false);
    btn_run.show(display);
    btn_setup.setFocus(true);
    btn_setup.show(display);
  }else{
    btn_run.setFocus(true);
    btn_run.show(display);
    btn_setup.setFocus(false);
    btn_setup.show(display);
  }
}
