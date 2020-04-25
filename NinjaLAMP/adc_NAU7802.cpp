#include "adc.h"
// #include "board_conf.h"
#include <Arduino.h>
#include <Wire.h>
//#include "pcr_includes.h"
#include "adc_NAU7802.h"

#ifdef USE_ADC_NAU7802
/* Skip init sequence and return dummy values. This mode is for testing board without */
// #define ADC_DUMMY_MODE 
#define NO_ERR 0x00
HardwareStatus switchADCConfig (uint8_t channel, uint8_t SPS0, uint8_t SPS1, uint8_t SPS2);
/* Implementation of NAU7802 A/D Converter */
char i2c_err;
// Basic communication

static char wellADCWriteRegValue(uint8_t reg_address, uint8_t b) {
  Wire.beginTransmission(NAU7802_DEVICE_ADDRESS); // transmit to device #0x2A
  Wire.write(reg_address);// sends 1 byte
  Wire.write(b);// sends 1 byte
  Wire.endTransmission();// stop transmitting
  return NO_ERR;
}
static char wellADCWriteRegValues (uint8_t reg_address, char *data, uint8_t dataSize) {
  int i;
  Wire.write(reg_address);
  for (i = 0; i < dataSize; i++) {
    Wire.write(data[i]);
  }
  // delay (1); //100 usec
  return NO_ERR;
}

// Read a single register value
static uint8_t wellADCReadRegValue(char reg_address) {
  Wire.beginTransmission(NAU7802_DEVICE_ADDRESS); // transmit to device #0x2A
  Wire.write((uint8_t)reg_address);// sends 1 byte
  Wire.endTransmission(false);// stop transmitting
  Wire.beginTransmission(NAU7802_DEVICE_ADDRESS);// transmit to device #0x2A
  Wire.requestFrom(NAU7802_DEVICE_ADDRESS, 1);// request 6 bytes from slave device #0x2A
  return Wire.read();
}
// Read continuous registers
static uint8_t wellADCReadRegValues(char reg_address, char *out, uint8_t dataSize) {
  Wire.beginTransmission(NAU7802_DEVICE_ADDRESS);
  Wire.write((uint8_t)reg_address);
  Wire.endTransmission(false);
  Wire.beginTransmission(NAU7802_DEVICE_ADDRESS);
  Wire.requestFrom(NAU7802_DEVICE_ADDRESS, dataSize);

  uint8_t reg_val = 0;
  int index = 0;
  while (Wire.available() && index < dataSize) {
    char c = Wire.read(); // receive a byte as character
    out[index++] = c;
  }
}

void clearRegisterBit (uint8_t reg_addr, int index) {
  uint8_t value = wellADCReadRegValue(reg_addr) & ~(1 << index);
  wellADCWriteRegValue(reg_addr, value);
}

void setRegisterBit (uint8_t reg_addr, int index) {
  uint8_t value = wellADCReadRegValue(reg_addr) | (1 << index);
  wellADCWriteRegValue(reg_addr, value);

}
static bool isAdcInitialized = false;
static uint8_t adc_default_conf = 0b0111100; //320sps (111)(OK)
//static uint8_t adc_default_conf = 0b0010100; //40sps (010)
//static uint8_t adc_default_conf = 0b0001100; //80sps (010)

/**
 * Wait till the value of the bit become specified value.
 * Returns
 * true: OK
 * false: Timeout
 */
bool waitForFlag (uint8_t regAddress, int flagIndex, bool flagValue, long timeoutMsec) {
  long startMillis = millis();
  char read_out[1] = {0xFF};
  long elapsed = 0;
  int count = 0;
  do {
    count++;
    wellADCReadRegValues(regAddress, &read_out[0], 1);
    bool flagResult = (read_out[0] & (0x01 << flagIndex))!=0;
    elapsed = (millis()-startMillis);
    if (elapsed < 0) {
      PCR_ADC_DEBUG_LINE("ELAPSED TIME IS NEGATIVE!");
      startMillis = millis(); // Reset
    }
    if (flagResult == flagValue) {
      /*
        Serial.print("e=");
        Serial.print(count);
        Serial.print(".");
        PCR_ADC_DEBUG_LINE(elapsed);
        */
        return true;
    }
  } while (elapsed < timeoutMsec);
  // Timeout
  Serial.print("TIMEOUT! resetting.");
  initADC();
  PCR_ADC_DEBUG_LINE(elapsed);
  delay(200);
  return false;
}

uint8_t initADC () {
  PCR_ADC_DEBUG_LINE("initADC");
  if (isAdcInitialized) {
    return 0;
  }
  delay(200);
  isAdcInitialized = true;

  Wire.begin(PIN_WELL_NAU7802_SDA, PIN_WELL_NAU7802_SCL); //sda, scl
  char read_out[1] = {0xFF};
  char write_init_block[1] = {0x16};
  char write_calib_block[1] = {0x04};
  char write_rate_block[1] = {adc_default_conf}; // 111=320SPS
  
  // Reset registers
  setRegisterBit(NAU7802_REG_ADDR_PU_CTRL, NAU7802_BIT_RR);
  clearRegisterBit(NAU7802_REG_ADDR_PU_CTRL, NAU7802_BIT_RR);
  // Power up digital
  setRegisterBit(NAU7802_REG_ADDR_PU_CTRL, NAU7802_BIT_PUD);
  // Wait for power up ready flag
  if (waitForFlag(NAU7802_REG_ADDR_PU_CTRL, NAU7802_BIT_PUR, true, 1000)) {
    PCR_ADC_DEBUG_LINE("initADC SUCCESS");
  } else {
    PCR_ADC_DEBUG_LINE("initADC FAIL");
    
  }
  // Power up analog
  setRegisterBit(NAU7802_REG_ADDR_PU_CTRL, NAU7802_BIT_PUA);
  // Cycle start
  setRegisterBit(NAU7802_REG_ADDR_PU_CTRL, NAU7802_BIT_PUA);
  wellADCWriteRegValue(NAU7802_REG_ADDR_CTRL2, adc_default_conf);

  // Set sampling rate
  HardwareStatus result = switchADCConfig(0, 0, 1, 0); //1ch, 40SPS
  
  // Start conversion
  setRegisterBit(NAU7802_REG_ADDR_CTRL2, NAU7802_BIT_CS);
  
  Serial.print("CTRL2=");
  PCR_ADC_DEBUG_LINE(wellADCReadRegValue(NAU7802_REG_ADDR_CTRL2));
  if (wellADCReadRegValue(NAU7802_REG_ADDR_REVISION)==0x0F) {
    PCR_ADC_DEBUG_LINE("Rev code OK");
  } else {
    PCR_ADC_DEBUG_LINE("Rev code WRONG");
  }
  return NO_ERR;
}

uint8_t prev_channel = 0;
void switchChannelTo (uint8_t channel) {
  if (channel != prev_channel) {
    // Switch channel
    if (channel == 1) {
      // 2ch (set bit 7)
      setRegisterBit(NAU7802_REG_ADDR_CTRL2, 7);
    } else {
      // 1ch (clear bit 7)
      clearRegisterBit(NAU7802_REG_ADDR_CTRL2, 7);
    }
    setRegisterBit(NAU7802_REG_ADDR_CTRL2, 2); //Calib
    prev_channel = channel;
  }
}

uint8_t changeBitValue (uint8_t value, uint8_t index, uint8_t bitValue) {
  if (bitValue) {
    // set
    return value | (1 << index);
  } else {
    // clear
    return value & ~(1 << index);
  }
}
HardwareStatus switchADCConfig (uint8_t channel, uint8_t SPS0, uint8_t SPS1, uint8_t SPS2) {
  // switchChannelTo(channel);
  uint8_t value = wellADCReadRegValue(NAU7802_REG_ADDR_CTRL2);
  value = changeBitValue(value, 7, (channel==1));
  // From right to left
  value = changeBitValue(value, 6, SPS2);
  value = changeBitValue(value, 5, SPS1);
  value = changeBitValue(value, 4, SPS0);
  wellADCWriteRegValue(NAU7802_REG_ADDR_CTRL2, value);
  HardwareStatus result;
  // Wait for "Calib OK" flag
  if (waitForFlag(NAU7802_REG_ADDR_CTRL2, 2, false, 500)==false) { return HARD_ERROR_ADC; }
  setRegisterBit(NAU7802_REG_ADDR_PU_CTRL, NAU7802_BIT_CS);
  return HARD_NO_ERROR;
}
float getADCValue () {
  uint32_t adc_val = 0xFFFFFF;
  char read_out[3] = {0xFF, 0xFF, 0xFF};
  i2c_err = wellADCReadRegValues(NAU7802_REG_ADDR_ADCO_B2, &read_out[0], 3);
  read_out[0] -= 0x80; // signed->unsigned
  adc_val = (read_out[0] << 16) | (read_out[1] << 8) | read_out[2];
  PCR_ADC_DEBUG("ADC=");
  PCR_ADC_DEBUG_LINE(String(adc_val,HEX));
  float ratio =  (float) adc_val / (1.0 * 0x1000000);
  return ratio;
}

/*
111 = 320SPS
011 = 80SPS
010 = 40SPS
001 = 20SPS
000 = 10SPS
*/
HardwareStatus getWellADCValue (float *val) {
#ifdef ADC_DUMMY_MODE
  return 0;
#endif /* ADC_DUMMY_MODE */
  // Wait for "Cycle ready" flag
  if (waitForFlag(NAU7802_REG_ADDR_PU_CTRL, NAU7802_BIT_CR, true, 500)==false) { return HARD_ERROR_ADC; }
  // Wait (if needed) Read -> save timestamp -> Switch Channel & Set SPS
  *val = getADCValue();
  HardwareStatus result = switchADCConfig(1, 0, 1, 0); //2ch, 40SPS
  delay(100);
  return result;
}

// Read ADC value of channel 1
HardwareStatus getLidADCValue (float *val) {
#ifdef ADC_DUMMY_MODE
  return 0;
#endif /* ADC_DUMMY_MODE */
  // Wait for "Cycle ready" flag
  if (waitForFlag(NAU7802_REG_ADDR_PU_CTRL, NAU7802_BIT_CR, true, 500)==false) { return HARD_ERROR_ADC; }
  // Wait (if needed) Read -> save timestamp -> Switch Channel & Set SPS
  *val = getADCValue();
  HardwareStatus result = switchADCConfig(0, 0, 1, 0); //1ch, 40SPS
  return result;
}

#endif /* USE_ADC_NAU7802 */
