#include "ADCArduino.h"
#include "NinjaLAMPCore.h"
#include "HardwareConf.h"

const int WELL_HEATER_PWM = 3;
const int WELL_THERMISTOR_AIN = A7;
const int AIR_THERMISTOR_AIN = A6;

/* Pin assignment of the first model (standalone) */
/*
const int WELL_HEATER_PWM = 15;
const int WELL_THERMISTOR_AIN = A0;
const int AIR_THERMISTOR_AIN = A1;
*/

/*
  Seeeduino XIAO model
  LED_R : PA02_A0_D0
  LED_G : PA04_A1_D1
  LED_B : PA10_A2_D2
  HEATER : PA11_A3_D3
  SDA : PA08_A4_D4_SDA
  SCL : PA09_A5_D5_SCL
  AIN_AIR : PB08_A6_D6_TX
  TACT2 : PA06_A10_D10_MOSI
  TACT1 : PA05_A9_D9_MISO
  TACT0 : PA07_A8_D8_SCK
  AIN_WELL : PB09_A7_D7_RX
*/
/* PID constants (Dependend on wells and heaters */
#define WELL_KP (0.11)
#define WELL_KI (0.5)
#define WELL_KD (2.0)

ADCArduino adc(WELL_THERMISTOR_AIN, AIR_THERMISTOR_AIN);

/* Well */
struct ThermistorRange wellThermistorRanges[3] = {
  { 0.0, 4250, 0.0, }, /* 4250 for 0-50 deg */
  { 50.0, 4311, 0.0, }, /* 4311 for 50-85 deg */
  { 85.0, 4334, 0.0, } /* 4334 for 85-100 deg */
};
Thermistor wellThermistor = { 
  .bConstRangeCount = 3, /* Number of B-constant ranges */
  .bConstRanges = wellThermistorRanges, /* Pointer to the B-constant ranges array */
  .r0 = 100.0, /* Resistance at "baseTemp" (kOhm) */
  .baseTemp = 25.0,
  .place = THERMISTOR_LOW_SIDE,
  .useSwitching = false,
  .r = 47.0 /* kOhm */
};
/* Air */
struct ThermistorRange airThermistorRanges[3] = {
  { 0.0, 4250, 0.0, }, /* 4250 for 0-50 deg */
  { 50.0, 4311, 0.0, }, /* 4311 for 50-85 deg */
  { 85.0, 4334, 0.0, } /* 4334 for 85-100 deg */
};
Thermistor airThermistor = { 
  .bConstRangeCount = 3, /* Number of B-constant ranges */
  .bConstRanges = airThermistorRanges, /* Pointer to the B-constant ranges array */
  .r0 = 100.0, /* Resistance at "baseTemp" (kOhm) */
  .baseTemp = 25.0,
  .place = THERMISTOR_LOW_SIDE,
  .useSwitching = false,
  .r = 47.0 /* kOhm */
};
NinjaLAMPCore * HardwareConf::initFirstModel () {
  NinjaLAMPCore *core = new NinjaLAMPCore(
    &wellThermistor,
    &airThermistor,
    &adc,
    WELL_KP, WELL_KI, WELL_KD,
    WELL_HEATER_PWM
  );
  /*
  NinjaLAMPCore core = {
    .wellThermistorConf = &wellThermistor, 
    .airThermistorConf = &airThermistor,
    .adc = &adc,
    .wellKP = WELL_KP,
    .wellKI = WELL_KI,
    .wellKD = WELL_KD,
    .heaterPWM = WELL_HEATER_PWM
  };
  */
  
  return core;
}
NinjaLAMPCore * HardwareConf::initLCDModel () {
  return NULL;
}
