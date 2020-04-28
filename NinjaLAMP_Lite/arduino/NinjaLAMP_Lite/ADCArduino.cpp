/* TODO for Arduino edition */
#include <Arduino.h>
#include "ADCArduino.h"

ADCArduino::ADCArduino (int ainWell, int ainAir) {
  analogInWell = ainWell;
  analogInAir = ainAir;
}
void ADCArduino::initADC () {
  
}
double ADCArduino::getWellADCValue () {
  return analogRead(analogInWell) / 1024.0;
  
}
double ADCArduino::getAirADCValue () {
  return analogRead(analogInAir) / 1024.0;
}
