#include "Arduino.h"
#include "NinjaLAMPCore.h"
#include "ADC.h"

#define KELVIN 273.15
#define INTERVAL_MSEC 250
#define TEMP_BUFF_SIZE 5

double setpoint, input, output;
double tempBuff[TEMP_BUFF_SIZE];
int tempBuffIndex = 0;
double targetTemp = TARGET_TEMP;

NinjaLAMPCore::NinjaLAMPCore (Thermistor *wellThermistorConf, Thermistor *airThermistorConf, ADCCustom *adc,
  double wellKP, double wellKI, double wellKD, int heaterPWM) {
  wellThermistor = wellThermistorConf;
  airThermistor = airThermistorConf;
  this->adc = adc;
  pid = new PID(&input, &output, &setpoint, wellKP, wellKI, wellKD, wellKD);
  this->heaterPWMPin = heaterPWM;
}
void NinjaLAMPCore::setup () {
  Serial.println("NinjaLAMPCore::startup");
  calcVoltageLimits(wellThermistor);
  calcVoltageLimits(airThermistor);
  pinMode(heaterPWMPin, OUTPUT);
  adc->initADC();
  if (wellThermistor->useSwitching) {
    pinMode(wellThermistor->switchingPin, OUTPUT);
    digitalWrite(wellThermistor->switchingPin, LOW);
  }
  input = readWellTemp();
  for (int i=0; i<TEMP_BUFF_SIZE; i++) {
    tempBuff[i] = input;
  }
  switchWellR(input);
  setupPID();
  delay(INTERVAL_MSEC/2);
}
void NinjaLAMPCore::loop () {
  if (started) {
    controlTemp();
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
  // Turn off well heater
  analogWrite(heaterPWMPin, 0);
}
/* Private functions */
void NinjaLAMPCore::controlTemp () {
  // Well temp
  double wellTempRaw = readWellTemp();
  tempBuff[tempBuffIndex] = wellTempRaw;
  switchWellR(wellTempRaw);
  tempBuffIndex = (tempBuffIndex + 1) % TEMP_BUFF_SIZE;
  double temp = averageTemp();
  if (temp < targetTemp - MAX_OUTPUT_THRESHOLD) {
    // Max drive
    analogWrite(heaterPWMPin, 1023);
    pid->SetMode(MANUAL);
  } else {
    // PID drive
    input = temp;
    pid->SetMode(AUTOMATIC);
    pid->Compute();
    double pwmOutput = (output + 0.5) * 408/*1024*/;
    analogWrite(heaterPWMPin, (int)pwmOutput);
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
void NinjaLAMPCore::setupPID () {
  setpoint = targetTemp;
  pid->SetMode(MANUAL);
  pid->SetOutputLimits(-0.5, 0.5);
  pid->SetSampleTime(INTERVAL_MSEC);
}

double NinjaLAMPCore::readWellTemp () {
  double wellVoltage = adc->getWellADCValue();
  double voltageRatio;
  if (wellThermistor->place == THERMISTOR_LOW_SIDE) {
    voltageRatio = wellVoltage;
  } else {
    voltageRatio = 1.0 - wellVoltage;
  }
  float bConstant = bConstantForVoltage(wellThermistor->bConstRangeCount, wellThermistor->bConstRanges, voltageRatio);
  return voltageToTemp(voltageRatio, wellThermistor->r, bConstant, wellThermistor->r0, wellThermistor->baseTemp);
}

double NinjaLAMPCore::readAirTemp () {
  double airVoltage = adc->getAirADCValue();
  double voltageRatio;
  if (airThermistor->place == THERMISTOR_LOW_SIDE) {
    voltageRatio = airVoltage;
  } else {
    voltageRatio = 1.0 - airVoltage;
  }
  float bConstant = bConstantForVoltage(airThermistor->bConstRangeCount, airThermistor->bConstRanges, voltageRatio);
  return voltageToTemp(voltageRatio, airThermistor->r, bConstant, airThermistor->r0, airThermistor->baseTemp);
}

double NinjaLAMPCore::bConstantForVoltage (int rangeCount, ThermistorRange *ranges, double voltageRatio) {
  float bConstant = ranges[0].bConst;
  for (int i=1; i<rangeCount; i++) {
    ThermistorRange *range = &ranges[i];
    if (voltageRatio < range->voltageLimit) {
      bConstant = range->bConst;
    }
  }
  return bConstant;
}
void NinjaLAMPCore::calcVoltageLimits (Thermistor *thermistor) {
  for (int i=1; i<thermistor->bConstRangeCount; i++) {
    ThermistorRange *range = &thermistor->bConstRanges[i];
    double vLimit = tempToVoltageRatio(range->tempLowerLimit, thermistor->r, 
      range->bConst, thermistor->r0, thermistor->baseTemp);
    range->voltageLimit = vLimit;
  }
}

void NinjaLAMPCore::switchWellR (double temp) {
  if (!wellThermistor->useSwitching) {
    return;
  }
  double prevValue = wellThermistor->r;
  if (temp > wellThermistor->switchingTemp) {
    wellThermistor->r = wellThermistor->rHigh;
    digitalWrite(wellThermistor->switchingPin, HIGH);
  } else {
    wellThermistor->r = wellThermistor->rLow;
    digitalWrite(wellThermistor->switchingPin, LOW);
  }
  if (wellThermistor->r != prevValue) {
    Serial.println("R SWITCH");
    calcVoltageLimits(wellThermistor);
  }
}

double NinjaLAMPCore::voltageToTemp (double voltageRatio, float resistance, 
  float b_constant, float r0, double baseTemp) {
  double thermistorR = resistance * voltageRatio / (1.0 - voltageRatio);
  return (1 / ((log(thermistorR / r0) / b_constant) + (1.0 / (baseTemp + KELVIN))))  - KELVIN;
}

double NinjaLAMPCore::tempToVoltageRatio (double tempCelsius, double resistance, 
  double bConst, double r0, double baseTemp) {
  double thermistorR = r0 * exp(bConst * (1 / (tempCelsius + KELVIN) - 1 / (baseTemp + KELVIN)));
  return thermistorR / (thermistorR + resistance);
}
double NinjaLAMPCore::averageTemp () {
  double tempSum = 0;
  for (int i=0; i<TEMP_BUFF_SIZE; i++) {
    tempSum += tempBuff[i];
  }
  return tempSum / TEMP_BUFF_SIZE;
}
