#include <Wire.h>

/*
 * Worked when I added delay(100) every line... (shingo 2020.Nov.13)
 * https://create.arduino.cc/projecthub/gatoninja236/how-to-use-i2c-eeprom-30767d
 */
 
#define ADDR_Ax 0b000 //A2, A1, A0
#define ADDR (0b1010 << 3) + ADDR_Ax

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(2000);
  Wire.begin();
  delay(2000);
  Serial.println("write start");
  writeI2CByte(0, 0x32);
  Serial.println("read start");
  Serial.println(readI2CByte(0)); // 0x32 = 50
  Serial.println("setup done");
}

void loop() {
  // put your main code here, to run repeatedly:
  
}

void writeI2CByte(byte data_addr, byte data){
  Wire.beginTransmission(ADDR);
  delay(100);
  Wire.write(data_addr);
  delay(100);
  Wire.write(data);
  delay(100);
  Wire.endTransmission();
  delay(100);
}

byte readI2CByte(byte data_addr){
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
