#include <Wire.h>

/**
 * A factory fresh EEPROM will have 0xFF (255) in all bytes si if 
 * 
 * The last byte of the EEPROM contains the value of the orientation_left_handed which is 0|1 
 * This value is used ny NinjaLAMPSysConfig to determine if the default configuration has been set.  
 * If the value is 0xFF then NinjaLAMPSysConfig::setup writes the default configuration.
 * 
 * Worked when I added delay(100) every line... (shingo 2020.Nov.13)
 * https://create.arduino.cc/projecthub/gatoninja236/how-to-use-i2c-eeprom-30767d
 */
 
#define ADDR_Ax 0b000 //A2, A1, A0
#define ADDR (0b1010 << 3) + ADDR_Ax

int x = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000);
  Wire.begin();
  delay(1000);

  Serial.print("Resetting last EEPROM byte...");
  writeI2CByte(255, 0xFF);
  Serial.println("done.");

  Serial.println("Reading default config");
  for (int i = 0 ; i < 4 ; i++) {
    Serial.print(readI2CByte(i));
    Serial.print(" ");
  }
  Serial.println();
  Serial.print("Last byte: ");
  Serial.print(readI2CByte(255));
  Serial.println();
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
