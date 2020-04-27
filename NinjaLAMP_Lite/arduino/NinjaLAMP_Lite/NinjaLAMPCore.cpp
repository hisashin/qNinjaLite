#include "Arduino.h"
#include "NinjaLAMPCore.h"
#include <PID_v1.h>
#include "adc_NAU7802.h"

/* Temperature COnfig */
#define MAX_OUTPUT_THRESHOLD 5

#define THERMISTOR_LOW_SIDE 1
#define THERMISTOR_HIGH_SIDE 2

/* Well */
const int wellThermistorRangeCount = 3;
struct ThermistorRange wellThermistorRanges[3] = {
  { 0.0, 4250, 0.0, },
  { 50.0, 4311, 0.0, },
  { 85.0, 4334, 0.0, },
};


double calcVoltageLimits (int rangeCount, ThermistorRange *ranges, double resistance, double r0, double baseTemp);
double readWellTemp ();
double readAirTemp ();

#define WELL_THERMISTOR_PLACE THERMISTOR_LOW_SIDE
#define WELL_R_0 100.0 // R0
#define WELL_THERMISTOR_BASE_TEMP 25.0
// #define WELL_R 47 // Counter resistor (breadboard test)

#define WELL_R_SWITCHING
#ifdef WELL_R_SWITCHING
#define PIN_WELL_HIGH_TEMP 16
#define WELL_R_LOW 30 // Counter resistor (Low mode)
#define WELL_R_HIGH 10 // Counter resistor (High mode)
#define WELL_R_SWITCHING_TEMP 54
#else
#define WELL_R 47 // Counter resistor
#endif

#ifdef WELL_R_SWITCHING
double wellR = WELL_R_LOW;
void switchWellR (double temp) {
  double prevValue = wellR;
  if (temp > WELL_R_SWITCHING_TEMP) {
    wellR = WELL_R_HIGH;
    digitalWrite(PIN_WELL_HIGH_TEMP, HIGH);
  } else {
    wellR = WELL_R_LOW;
    digitalWrite(PIN_WELL_HIGH_TEMP, LOW);
  }
  if (wellR != prevValue) {
    Serial.println("R SWITCH");
    calcVoltageLimits(wellThermistorRangeCount, wellThermistorRanges, wellR, WELL_R_0, WELL_THERMISTOR_BASE_TEMP);
  }
}
#else
double wellR = WELL_R;
#endif

/* Air */
const int airThermistorRangeCount = 3;
struct ThermistorRange airThermistorRanges[3] = {
  { 0.0, 4250, 0.0, },
  { 50.0, 4311, 0.0, },
  { 85.0, 4334, 0.0, },
};
#define AIR_THERMISTOR_PLACE THERMISTOR_HIGH_SIDE
#define AIR_R_0 100.0 // R0
#define AIR_THERMISTOR_BASE_TEMP 25.0
#define AIR_R 4.99 // Counter resistor
// #define AIR_R 47 // Counter resistor (breadboard test)

#define USE_EXTERNAL_ADC

/* Pinouts */
const int WELL_HEATER_PWM = 15;

/* Define analog input pins to use Arduino's AIN pins */
#ifndef USE_EXTERNAL_ADC
const int WELL_THERMISTOR_AIN = A0;
const int AIR_THERMISTOR_AIN = A1;
#endif

/* PID constants */
#define WELL_KP (0.11)
#define WELL_KI (0.5)
#define WELL_KD (2.0)

#define KELVIN 273.15
double setpoint, input, output;
// https://playground.arduino.cc/Code/PIDLibraryConstructor/
PID pid(&input, &output, &setpoint, WELL_KP, WELL_KI, WELL_KD, DIRECT);


#define INTERVAL_MSEC 250
#define TEMP_BUFF_SIZE 5
double tempBuff[TEMP_BUFF_SIZE];
int tempBuffIndex = 0;

double voltageToTemp (double voltageRatio, float resistance, float b_constant, float r0, double baseTemp) {
  double thermistorR = resistance * voltageRatio / (1.0 - voltageRatio);
  return (1 / ((log(thermistorR / r0) / b_constant) + (1.0 / (baseTemp + KELVIN))))  - KELVIN;
}

double tempToVoltageRatio (double tempCelsius, double resistance, double bConst, double r0, double baseTemp) {
  double thermistorR = r0 * exp(bConst * (1 / (tempCelsius + KELVIN) - 1 / (baseTemp + KELVIN)));
  return thermistorR / (thermistorR + resistance);
}
double averageTemp () {
  double tempSum = 0;
  for (int i=0; i<TEMP_BUFF_SIZE; i++) {
    tempSum += tempBuff[i];
  }
  return tempSum / TEMP_BUFF_SIZE;
}
  
double prev = 0;
double targetTemp = TARGET_TEMP;
double calcVoltageLimits (int rangeCount, ThermistorRange *ranges, double resistance, double r0, double baseTemp) {
  for (int i=1; i<rangeCount; i++) {
    ThermistorRange *range = &ranges[i];
    double vLimit = tempToVoltageRatio(range->tempLowerLimit, resistance, range->bConst, r0, baseTemp);
    range->voltageLimit = vLimit;
  }
}

void setupPID () {
  setpoint = targetTemp;
  pid.SetMode(MANUAL);
  pid.SetOutputLimits(-0.5, 0.5);
  pid.SetSampleTime(INTERVAL_MSEC);
  
}
void setupCore () {
  calcVoltageLimits(wellThermistorRangeCount, wellThermistorRanges, wellR, WELL_R_0, WELL_THERMISTOR_BASE_TEMP);
  calcVoltageLimits(airThermistorRangeCount, airThermistorRanges, AIR_R, AIR_R_0, AIR_THERMISTOR_BASE_TEMP);
  pinMode(WELL_HEATER_PWM, OUTPUT);
#ifdef WELL_R_SWITCHING
  pinMode(PIN_WELL_HIGH_TEMP, OUTPUT);
  digitalWrite(PIN_WELL_HIGH_TEMP, LOW);
#endif
  input = readWellTemp();
  for (int i=0; i<TEMP_BUFF_SIZE; i++) {
    tempBuff[i] = input;
  }

#ifdef WELL_R_SWITCHING
  switchWellR(input);
#endif
  setupPID();
  delay(INTERVAL_MSEC/2);
}


void loopCore () {
  // Well temp
  double wellTempRaw = readWellTemp();
  tempBuff[tempBuffIndex] = wellTempRaw;
#ifdef WELL_R_SWITCHING
  switchWellR(wellTempRaw);
#endif
  tempBuffIndex = (tempBuffIndex + 1) % TEMP_BUFF_SIZE;
  double temp = averageTemp();
  if (temp < targetTemp - MAX_OUTPUT_THRESHOLD) {
    // Max drive
    analogWrite(WELL_HEATER_PWM, 1023);
    pid.SetMode(MANUAL);
  } else {
    // PID drive
    input = temp;
    pid.SetMode(AUTOMATIC);
    pid.Compute();
    double pwmOutput = (output + 0.5) * 408/*1024*/;
    analogWrite(WELL_HEATER_PWM, (int)pwmOutput);
  }
  // TODO: switch high/low resistors if needed
  delay(INTERVAL_MSEC/2);

  // Air temp
  double airTemp = readAirTemp();
  Serial.print(airTemp);
  Serial.print("\t");
  Serial.println(temp);
  delay(INTERVAL_MSEC/2);  
}


/* Read analog value */
double readWellThermistorVoltageRatio () {
#ifdef USE_EXTERNAL_ADC
  float voltageRatio;
  HardwareStatus result = getWellADCValue(&voltageRatio);
  return voltageRatio;
#else
  return analogRead(WELL_THERMISTOR_AIN) / 1024.0;
#endif
}

double readAirThermistorVoltageRatio () {
#ifdef USE_EXTERNAL_ADC
  float voltageRatio;
  HardwareStatus result = getLidADCValue(&voltageRatio);
  return voltageRatio;
#else
  return analogRead(AIR_THERMISTOR_AIN) / 1024.0;
#endif
  
}

double bConstantForVoltage (int rangeCount, ThermistorRange *ranges, double voltageRatio) {
  float bConstant = ranges[0].bConst;
  for (int i=1; i<rangeCount; i++) {
    ThermistorRange *range = &ranges[i];
    if (voltageRatio < range->voltageLimit) {
      bConstant = range->bConst;
    }
  }
  return bConstant;
}

double readWellTemp () {
  double wellVoltage = readWellThermistorVoltageRatio();
#if WELL_THERMISTOR_PLACE == 1
  double voltageRatio = wellVoltage;
#else
  double voltageRatio = 1.0 - wellVoltage;
#endif
  float bConstant = bConstantForVoltage(wellThermistorRangeCount, wellThermistorRanges, voltageRatio);
  return voltageToTemp(voltageRatio, wellR, bConstant, WELL_R_0, WELL_THERMISTOR_BASE_TEMP);
}

double readAirTemp () {
  double airVoltage = readAirThermistorVoltageRatio();
#if AIR_THERMISTOR_PLACE==THERMISTOR_LOW_SIDE
  double voltageRatio = airVoltage;
#else
  double voltageRatio = 1.0 - airVoltage;
#endif
  float bConstant = bConstantForVoltage(airThermistorRangeCount, airThermistorRanges, voltageRatio);
  return voltageToTemp(voltageRatio, AIR_R, bConstant, AIR_R_0, AIR_THERMISTOR_BASE_TEMP);
}

NinjaLAMPCore::NinjaLAMPCore () {
  
}
void NinjaLAMPCore::setup () {
  Serial.println("NinjaLAMPCore::startup");
  setupCore();
  
}
void NinjaLAMPCore::loop () {
  if (started) {
    loopCore();
  } else {
    delay(100);
  }
}
void NinjaLAMPCore::start (double temp) {
  targetTemp = temp;
  setupPID();
  started = true;
}
void NinjaLAMPCore::stop () {
  started = false;
  analogWrite(WELL_HEATER_PWM, 0);
}
