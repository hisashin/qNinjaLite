#include "ADCArduino.h"
#include "HardwareConf.h"

struct ThermistorRange NXFT15WF104FA2B100[3] = {
  { 0.0, 4250, 0.0, }, /* 4250 for 0-50 deg */
  { 50.0, 4311, 0.0, }, /* 4311 for 50-85 deg */
  { 85.0, 4334, 0.0, } /* 4334 for 85-100 deg */
};

struct ThermistorRange NTCG164KF104FT1S[4] = {
  { 0.0, 4419, 0.0, }, /* 4419 for 25-50 deg */
  { 50.0, 4468, 0.0, }, /* 4468 for 50-75 deg */
  { 75.0, 4485, 0.0, }, /* 4485 for 75-85 deg */
  { 85.0, 4509, 0.0, } /* 4509 for 85-100 deg */
};

NinjaLAMPCore * HardwareConf::initFirstModel () {
  /* PID constants (Dependend on wells and heaters */
  double WELL_KP = 0.11;
  double WELL_KI = 0.5;
  double WELL_KD = 2.0;

  /* Pin assignment of the first model */
  int WELL_HEATER_PWM = 15;
  int WELL_THERMISTOR_AIN = A0;
  int AIR_THERMISTOR_AIN = A1;
  
  ADCArduino *adc = new ADCArduino(WELL_THERMISTOR_AIN, AIR_THERMISTOR_AIN);
  Thermistor *wellThermistor = new Thermistor{ 
    .bConstRangeCount = 3, /* Number of B-constant ranges */
    .bConstRanges = NXFT15WF104FA2B100, /* Pointer to the B-constant ranges array */
    .r0 = 100.0, /* Resistance at "baseTemp" (kOhm) */
    .baseTemp = 25.0,
    .place = THERMISTOR_LOW_SIDE,
    .useSwitching = false,
    .r = 47.0 /* kOhm */
  };
  Thermistor *airThermistor = new Thermistor{ 
    .bConstRangeCount = 3, /* Number of B-constant ranges */
    .bConstRanges = NXFT15WF104FA2B100, /* Pointer to the B-constant ranges array */
    .r0 = 100.0, /* Resistance at "baseTemp" (kOhm) */
    .baseTemp = 25.0,
    .place = THERMISTOR_LOW_SIDE,
    .useSwitching = false,
    .r = 47.0 /* kOhm */
  };
  NinjaLAMPCore *core = new NinjaLAMPCore(
    wellThermistor,
    airThermistor,
    adc,
    WELL_KP, WELL_KI, WELL_KD,
    WELL_HEATER_PWM
  );
  return core;
}
NinjaLAMPCore * HardwareConf::initLCDModel () {
  /* PID constants (Dependend on wells and heaters */
  double WELL_KP = 0.11;
  double WELL_KI = 0.5;
  double WELL_KD = 2.0;

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
  int WELL_HEATER_PWM = 3;
  int WELL_THERMISTOR_AIN = A7;
  int AIR_THERMISTOR_AIN = A6;
  
  int PIN_LED_R = 0;
  int PIN_LED_G = 1;
  int PIN_LED_B = 2;
  int PIN_I2C_SDA = 4;
  int PIN_I2C_SCL = 5;
  int PIN_TACT0 = 8;
  int PIN_TACT1 = 9;
  int PIN_TACT2 = 810;
  
  ADCArduino *adc = new ADCArduino(WELL_THERMISTOR_AIN, AIR_THERMISTOR_AIN);
  Thermistor *wellThermistor = new Thermistor{ 
    .bConstRangeCount = 3, /* Number of B-constant ranges */
    .bConstRanges = NXFT15WF104FA2B100, /* Pointer to the B-constant ranges array */
    .r0 = 100.0, /* Resistance at "baseTemp" (kOhm) */
    .baseTemp = 25.0,
    .place = THERMISTOR_LOW_SIDE,
    .useSwitching = false,
    .r = 47.0 /* kOhm */
  };
  Thermistor *airThermistor = new Thermistor{ 
    .bConstRangeCount = 4, /* Number of B-constant ranges */
    .bConstRanges = NTCG164KF104FT1S, /* Pointer to the B-constant ranges array */
    .r0 = 100.0, /* Resistance at "baseTemp" (kOhm) */
    .baseTemp = 25.0,
    .place = THERMISTOR_LOW_SIDE,
    .useSwitching = false,
    .r = 47.0 /* kOhm */
  };
  NinjaLAMPCore *core = new NinjaLAMPCore(
    wellThermistor,
    airThermistor,
    adc,
    WELL_KP, WELL_KI, WELL_KD,
    WELL_HEATER_PWM
  );
  return core;
}
