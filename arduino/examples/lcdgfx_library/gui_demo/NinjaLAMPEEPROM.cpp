#include "NinjaLAMPEEPROM.h"

#define DEBUG_EEPROM 1

// Constructor
NinjaLAMPEEPROM::NinjaLAMPEEPROM () {

}

void NinjaLAMPEEPROM::setup() {
  #ifdef DEBUG
    Serial.println("NinjaLAMPEEPROM setup");
  #endif
  Wire.begin();
  delay(2000);
}

void NinjaLAMPEEPROM::writeI2CArray(byte data_addr, byte* data, byte len){
  #ifdef DEBUG_EEPROM
    Serial.print("NinjaLAMPEEPROM::writeI2CArray - data_addr: ");
    Serial.print(data_addr);
    Serial.print(" - len: ");
    Serial.println(len);
  #endif
  Wire.beginTransmission(ADDR);
  delay(100);
  Wire.write(data_addr);
  delay(100);
  Wire.write(data, len);
  delay(100);
  Wire.endTransmission();
  delay(100);
}

void NinjaLAMPEEPROM::writeI2CByte(byte data_addr, byte data){
  #ifdef DEBUG_EEPROM
    Serial.print("NinjaLAMPEEPROM::writeI2CByte - data_addr: ");
    Serial.print(data_addr);
    Serial.print(" - data: ");
    Serial.println(data);
  #endif
  Wire.beginTransmission(ADDR);
  delay(100);
  Wire.write(data_addr);
  delay(100);
  Wire.write(data);
  delay(100);
  Wire.endTransmission();
  delay(100);
}

byte NinjaLAMPEEPROM::readI2CByte(byte data_addr){
  #ifdef DEBUG_EEPROM
    Serial.print("NinjaLAMPEEPROM::readI2CByte - data_addr: ");
    Serial.println(data_addr);
  #endif
  byte data = NULL;
  Wire.beginTransmission(ADDR);
  delay(100);
  Wire.write(data_addr);
  delay(100);
  Wire.endTransmission();
  delay(100);
  Wire.requestFrom(ADDR, 1); //retrieve 1 returned byte
  delay(100);
  delay(1);
  if(Wire.available()){
  delay(100);
    data = Wire.read();
  }
  delay(100);
  return data;
}
