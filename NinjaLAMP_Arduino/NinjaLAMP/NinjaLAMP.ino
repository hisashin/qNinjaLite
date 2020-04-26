#include <PID_v1.h>
#include "adc_NAU7802.h"

/* Temperature COnfig */
#define TARGET_TEMP 63
#define MAX_OUTPUT_THRESHOLD 5

struct ThermistorRange {
  double tempLowerLimit;
  int bConst;
  double voltageLimit;
};
#define THERMISTOR_LOW_SIDE 1
#define THERMISTOR_HIGH_SIDE 2

/* Well */
const int wellThermistorRangeCount = 3;
struct ThermistorRange wellThermistorRanges[3] = {
  { 0.0, 4250, 0.0, },
  { 50.0, 4311, 0.0, },
  { 85.0, 4334, 0.0, },
};
const int WELL_THERMISTOR_PLACE = THERMISTOR_LOW_SIDE;
#define WELL_R_0 100.0 // R0
#define WELL_THERMISTOR_BASE_TEMP 25.0
#define WELL_R 30 // Counter resistor
// #define WELL_R 47 // Counter resistor (breadboard test)

/* Air */
const int airThermistorRangeCount = 3;
struct ThermistorRange airThermistorRanges[3] = {
  { 0.0, 4250, 0.0, },
  { 50.0, 4311, 0.0, },
  { 85.0, 4334, 0.0, },
};
const int AIR_THERMISTOR_PLACE = THERMISTOR_HIGH_SIDE;
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
static double SWITCHING_VOLTAGE_50 = 0;
// static float INVERSE_BASE_TEMP = 1.0 / (THERMISTOR_BASE_TEMP + KELVIN);

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
void setup() {
  Serial.begin(9600);
  for (int i=1; i<wellThermistorRangeCount; i++) {
    ThermistorRange *range = &wellThermistorRanges[i];
    double vLimit = tempToVoltageRatio(range->tempLowerLimit, WELL_R, range->bConst, WELL_R_0, WELL_THERMISTOR_BASE_TEMP);
    range->voltageLimit = vLimit;
  }
  for (int i=1; i<airThermistorRangeCount; i++) {
    ThermistorRange *range = &airThermistorRanges[i];
    double vLimit = tempToVoltageRatio(range->tempLowerLimit, WELL_R, range->bConst, WELL_R_0, AIR_THERMISTOR_BASE_TEMP);
    range->voltageLimit = vLimit;
  }
  pinMode(WELL_HEATER_PWM, OUTPUT);
  input = readWellTemp();
  for (int i=0; i<TEMP_BUFF_SIZE; i++) {
    tempBuff[i] = input;
  }
  setpoint = TARGET_TEMP;
  pid.SetMode(MANUAL);
  pid.SetOutputLimits(-0.5, 0.5);
  pid.SetSampleTime(INTERVAL_MSEC);
  delay(INTERVAL_MSEC/2);
}

double prevTemp = 0;
void loop() {
  // Well temp
  tempBuff[tempBuffIndex] = readWellTemp();
  tempBuffIndex = (tempBuffIndex + 1) % TEMP_BUFF_SIZE;
  double temp = averageTemp();
  if (temp < TARGET_TEMP - MAX_OUTPUT_THRESHOLD) {
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
  //Serial.println(temp);
  delay(INTERVAL_MSEC/2);
  prevTemp = temp;

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
  // TODO: define normal/reverse voltages
#if WELL_THERMISTOR_PLACE==THERMISTOR_LOW_SIDE
  double voltageRatio = wellVoltage;
#else
  double voltageRatio = 1.0 - wellVoltage;
#endif
  float bConstant = bConstantForVoltage(wellThermistorRangeCount, wellThermistorRanges, voltageRatio);
  return voltageToTemp(voltageRatio, WELL_R, bConstant, WELL_R_0, WELL_THERMISTOR_BASE_TEMP);
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
