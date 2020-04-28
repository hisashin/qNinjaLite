#include <EEPROM.h>
#include "NinjaLAMPCore.h"


/* Well */
struct ThermistorRange wellThermistorRanges[3] = {
  { 0.0, 4250, 0.0, }, /* 4250 for 0-50 deg */
  { 50.0, 4311, 0.0, }, /* 4311 for 50-85 deg */
  { 85.0, 4334, 0.0, } /* 4334 for 85-100 deg */
};
Thermistor wellThermistor = { 
  .bConstRangeCount = 3,
  .bConstRanges = wellThermistorRanges, 
  .r0 = 100.0,
  .baseTemp = 25.0,
  .place = THERMISTOR_LOW_SIDE,
  /*
   .useSwitching = false,
   .r = 47.0
   */
  .useSwitching = true,
  .r = 30.0,
  .rLow = 30.0,
  .rHigh = 10.0,
  .switchingTemp = 54.0,
  .switchingPin = 0
};
/* Air */
struct ThermistorRange airThermistorRanges[3] = {
  { 0.0, 4250, 0.0, }, /* 4250 for 0-50 deg */
  { 50.0, 4311, 0.0, }, /* 4311 for 50-85 deg */
  { 85.0, 4334, 0.0, } /* 4334 for 85-100 deg */
};
Thermistor airThermistor = { 
  .bConstRangeCount = 3,
  .bConstRanges = airThermistorRanges, 
  .r0 = 100.0,
  .baseTemp = 25.0,
  .place = THERMISTOR_HIGH_SIDE,
  .useSwitching = false,
  .r = 4.99
  // .r = 47.0
};

/* Pinouts */
const int WELL_HEATER_PWM = 15;

/* PID constants */
#define WELL_KP (0.11)
#define WELL_KI (0.5)
#define WELL_KD (2.0)

#define USE_EXTERNAL_ADC
#ifdef USE_EXTERNAL_ADC
#include "adc_NAU7802.h"
ADCNAU7802 adc;
#else
#include "ADCArduino.h"
/* Define analog input pins to use Arduino's AIN pins */
const int WELL_THERMISTOR_AIN = A0;
const int AIR_THERMISTOR_AIN = A1;
ADCArduino adc(WELL_THERMISTOR_AIN, AIR_THERMISTOR_AIN);
#endif

NinjaLAMPCore core = {
  .wellThermistorConf = &wellThermistor, 
  .airThermistorConf = &airThermistor,
  .adc = &adc,
  .wellKP = WELL_KP,
  .wellKI = WELL_KI,
  .wellKD = WELL_KD,
  .heaterPWM = WELL_HEATER_PWM
  };

#define USE_WIFI /* Use WiFi functionalities */
#define PIN_WIFI_MODE 16 /* Digitai input pin to switch AP (pairing) mode and Normal mode  */
#define VALUE_WIFI_MODE_AP HIGH /* Start with AP (pairing) mode if value of PIN_WIFI_MODE agrees with this.  */
bool isApMode = false;
void setup() {
  Serial.begin(9600);
  Serial.println("NinjaLAMP");
  EEPROM.begin(1024);
  pinMode(PIN_WIFI_MODE, INPUT);
  delay(100);
  isApMode = (digitalRead(PIN_WIFI_MODE)==VALUE_WIFI_MODE_AP);
  if (isApMode) {
    Serial.println("AP mode");
    startWiFiAPMode();
    
  } else {
    Serial.println("Core mode");
    startWiFiHTTPServerMode();
  }
  core.setup();
}

void loop() {
  if (isWiFiConnected()) {
      loopWiFiHTTPServer();
  }
  core.loop();
}
