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

#define EEPROM_DELAY 5

void NinjaLAMPEEPROM::writeI2CArray(byte data_addr, byte* data, byte len){
  #ifdef DEBUG_EEPROM
    Serial.print("NinjaLAMPEEPROM::writeI2CArray - data_addr: ");
    Serial.print(data_addr);
    Serial.print(" - len: ");
    Serial.println(len);
  #endif
  Wire.beginTransmission(ADDR);
  delay(EEPROM_DELAY);
  Wire.write(data_addr);
  delay(EEPROM_DELAY);
  Wire.write(data, len);
  delay(EEPROM_DELAY);
  Wire.endTransmission();
  delay(EEPROM_DELAY);
}

void NinjaLAMPEEPROM::writeI2CByte(byte data_addr, byte data){
  #ifdef DEBUG_EEPROM
    Serial.print("NinjaLAMPEEPROM::writeI2CByte - data_addr: ");
    Serial.print(data_addr);
    Serial.print(" - data: ");
    Serial.println(data);
  #endif
  Wire.beginTransmission(ADDR);
  delay(EEPROM_DELAY);
  Wire.write(data_addr);
  delay(EEPROM_DELAY);
  Wire.write(data);
  delay(EEPROM_DELAY);
  Wire.endTransmission();
  delay(EEPROM_DELAY);
}

byte NinjaLAMPEEPROM::readI2CByte(byte data_addr){
  #ifdef DEBUG_EEPROM
    Serial.print("NinjaLAMPEEPROM::readI2CByte - data_addr: ");
    Serial.println(data_addr);
  #endif
  byte data = NULL;
  Wire.beginTransmission(ADDR);
  delay(EEPROM_DELAY);
  Wire.write(data_addr);
  delay(EEPROM_DELAY);
  Wire.endTransmission();
  delay(EEPROM_DELAY);
  Wire.requestFrom(ADDR, 1); //retrieve 1 returned byte
  delay(EEPROM_DELAY);
  if(Wire.available()){
  delay(EEPROM_DELAY);
    data = Wire.read();
  }
  delay(EEPROM_DELAY);
  return data;
}
