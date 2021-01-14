#include "NinjaLAMPSysConfig.h"
#include "NinjaLAMPEEPROM.h"

#define DEBUG_CONFIG 1

NinjaLAMPEEPROM eeprom;

// Constructor
NinjaLAMPSysConfig::NinjaLAMPSysConfig () {

}

void NinjaLAMPSysConfig::setup () {
  #ifdef DEBUG
    Serial.println("NinjaLAMPSysConfig::setup");
  #endif
  // Initialize the wire library.
  eeprom.setup();
  // Write the default values if not set. All the bytes of a new EEPROM will be set to 0xFF.
  orientation_left_handed = eeprom.readI2CByte(EEPROM_ORIENTATION_LEFT_HANDED);
  #ifdef DEBUG_CONFIG
    Serial.print("NinjaLAMPSysConfig::setup orientation_left_handed: ");
    Serial.println(orientation_left_handed);
  #endif
  if(orientation_left_handed == 0xFF){
    #ifdef DEBUG_CONFIG
      Serial.println("NinjaLAMPSysConfig::setup writing the default configuration...");
    #endif    
    // Write the default configuration.
    orientation_left_handed = 0;
    eeprom.writeI2CByte(EEPROM_ORIENTATION_LEFT_HANDED, 0);
    // Fill in the default cycle following the Cycle structure layout.
    byte cycleArray[EEPROM_CYCLES_LEN] = {(byte) 65, (byte) 30, (byte) 85, (byte) 2};
    eeprom.writeI2CArray(EEPROM_CYCLES_START, cycleArray, EEPROM_CYCLES_LEN);
    eeprom.writeI2CByte(EEPROM_CYCLES_COUNT, 1);
    #ifdef DEBUG_CONFIG
      Serial.println("NinjaLAMPSysConfig::setup DONE writing the default configuration.");
    #endif  
  }
  // Read the configuration into the public variables.
  cycleMaxCount = eeprom.readI2CByte(EEPROM_CYCLES_MAX_COUNT);
  cycleCount = eeprom.readI2CByte(EEPROM_CYCLES_COUNT);
  set_cycles();
  #ifdef DEBUG_CONFIG
    Serial.print("NinjaLAMPSysConfig::setup cycleMaxCount: ");
    Serial.println(cycleMaxCount);
    Serial.print("NinjaLAMPSysConfig::setup cycleCount: ");
    Serial.println(cycleCount);
  #endif
}

/**
 * Private method to read the cucles from the EEPROM into the public array cycles of type Cycle.
 */
void NinjaLAMPSysConfig::set_cycles() {
  // Resize the array.
  if (cycles != 0) {
    delete [] cycles;
  }
  cycles = new Cycle [cycleCount];
  // Fill the array.
  for(int i = 0; i < cycleCount; i++){
    int offset = i * EEPROM_CYCLES_LEN;
    Cycle cycle;
    cycle.amplifyTemp = eeprom.readI2CByte(EEPROM_CYCLES_START + offset);
    cycle.amplifyTime = eeprom.readI2CByte(EEPROM_CYCLES_START + offset + 1);
    cycle.stopTemp = eeprom.readI2CByte(EEPROM_CYCLES_START + offset + 2);
    cycle.stopTime = eeprom.readI2CByte(EEPROM_CYCLES_START + offset + 3);
    sprintf(cycle.menuItem, "%dc%dm-%dc%dm", cycle.amplifyTemp, cycle.amplifyTime, cycle.stopTemp, cycle.stopTime); 

//    cycle.menuItem = 
    #ifdef DEBUG_CONFIG
      Serial.print("NinjaLAMPSysConfig::get_cycles cycle ");
      Serial.println(i);      
      Serial.print("NinjaLAMPSysConfig::get_cycles amplifyTemp: ");
      Serial.println(cycle.amplifyTemp);
      Serial.print("NinjaLAMPSysConfig::get_cycles amplifyTime: ");
      Serial.println(cycle.amplifyTime);
      Serial.print("NinjaLAMPSysConfig::get_cycles stopTemp: ");
      Serial.println(cycle.stopTemp);
      Serial.print("NinjaLAMPSysConfig::get_cycles stopTime: ");
      Serial.println(cycle.stopTime); 
    #endif
    cycles[i] = cycle;
  }
}
