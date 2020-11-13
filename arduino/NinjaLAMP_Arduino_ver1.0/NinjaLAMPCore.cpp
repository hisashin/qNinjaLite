#include "Arduino.h"
#include "NinjaLAMPCore.h"
#include "adc.h"

#define KELVIN 273.15
#define INTERVAL_MSEC 250
#define TEMP_BUFF_SIZE 5

#define HOLDING_TEMP_TOLERANCE 0.5

double setpoint, input, output;
double wellTempBuff[TEMP_BUFF_SIZE];
int wellTempBuffIndex = 0;

double targetTemp = TARGET_TEMP;

NinjaLAMPCore::NinjaLAMPCore (Thermistor *wellThermistorConf, Thermistor *airThermistorConf, ADCCustom *adc,
  double wellKP, double wellKI, double wellKD, int heaterPWM) {
  wellThermistor = wellThermistorConf;
  airThermistor = airThermistorConf;
  this->adc = adc;
  totalElapsedTime = 0;
  stageElapsedTime = 0;
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
  wellTemp = readWellTemp();
  estimatedSampleTemp = wellTemp;
  switchWellR(wellTemp);
  setupPID();
  delay(INTERVAL_MSEC/2);
  readAirTemp();
  delay(INTERVAL_MSEC/2);
}

void NinjaLAMPCore::enableSampleTempSimulation (double heatResistanceRatio, double sampleHeatCapacity) {
  this->isSampleTempSimulationEnabled = true;
  this->heatResistanceRatio = heatResistanceRatio;
  this->sampleHeatCapacity = sampleHeatCapacity;
}
void NinjaLAMPCore::disableSampleTempSimulation () {
  isSampleTempSimulationEnabled = false;
}

void NinjaLAMPCore::start (double temp) {
  targetTemp = temp;
  setupPID();
  // Fill well temp buff
  input = readWellTemp();
  for (int i=0; i<TEMP_BUFF_SIZE; i++) {
    wellTempBuff[i] = input;
  }
  delay(INTERVAL_MSEC/2);
  // To switch ADC channel
  readAirTemp();
  started = true;
  lastTimestamp = millis();
}
void NinjaLAMPCore::setTargetTemp(double temp) {
  targetTemp = temp;
  isHolding = false;
  stageElapsedTime = 0;
  setupPID();
}
void NinjaLAMPCore::stop () {
  started = false;
  // Turn off well heater
  analogWrite(heaterPWMPin, 0);
  isHolding = false;
  targetTemp = airTemp;
  setpoint = airTemp;
  stageElapsedTime = 0;
}
/* Getter */
double NinjaLAMPCore::getWellTemp() {
  return wellTemp;
}
double NinjaLAMPCore::getAirTemp() {
  return airTemp;
}
double NinjaLAMPCore::getEstimatedSampleTemp() {
  return estimatedSampleTemp;
}
double NinjaLAMPCore::getTargetTemp() {
  return targetTemp;
}
double NinjaLAMPCore::getTempSetpoint() {
  return setpoint;
}
unsigned long NinjaLAMPCore::getTotalElapsedTime() {
  return totalElapsedTime;
}
unsigned long NinjaLAMPCore::getStageElapsedTime() {
  return stageElapsedTime;
}
void NinjaLAMPCore::loop () {
  // Well temp
  double wellTempRaw = readWellTemp();
  wellTempBuff[wellTempBuffIndex] = wellTempRaw;
  switchWellR(wellTempRaw);
  wellTempBuffIndex = (wellTempBuffIndex + 1) % TEMP_BUFF_SIZE;
  wellTemp = averageTemp();
  
  if (started) {
    controlTemp();
  }
  delay(INTERVAL_MSEC/2);

  // Air temp
  airTemp = readAirTemp();
  if (isSampleTempSimulationEnabled) {
    // Update setpoint according to air & target temp
    double diff = (INTERVAL_MSEC/1000.0) 
      * ((wellTemp - estimatedSampleTemp)/1.0 + (airTemp-estimatedSampleTemp)/heatResistanceRatio ) 
      / sampleHeatCapacity;
    if (5 > diff && diff > -5) {
      estimatedSampleTemp += diff;
    }
    if (airTemp < wellTemp) {
      setpoint = targetTemp + (targetTemp - airTemp) / heatResistanceRatio;
    }
  } else {
    estimatedSampleTemp = wellTemp;
  }
  if (!isHolding && abs(estimatedSampleTemp - targetTemp) < HOLDING_TEMP_TOLERANCE) {
    isHolding = true;
  } 
  delay(INTERVAL_MSEC/2);
  unsigned long timestamp = millis();
  unsigned long elapsed;
  if (timestamp > lastTimestamp) {
    elapsed = timestamp - lastTimestamp;
  } else {
    // In case of overflow
    elapsed = INTERVAL_MSEC;
  }
  totalElapsedTime += elapsed;
  if (isHolding) {
    stageElapsedTime += elapsed;
  }
  lastTimestamp = timestamp;
}

/* Private functions */
void NinjaLAMPCore::controlTemp () {
  if (wellTemp < targetTemp - MAX_OUTPUT_THRESHOLD) {
    // Max drive
    analogWrite(heaterPWMPin, 1023);
    pid->SetMode(MANUAL);
  } else {
    // PID drive
    input = wellTemp;
    pid->SetMode(AUTOMATIC);
    pid->Compute();
    double pwmOutput = (output + 0.5) * 408/*1024*/;
    analogWrite(heaterPWMPin, (int)pwmOutput);
  }
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
    tempSum += wellTempBuff[i];
  }
  return tempSum / TEMP_BUFF_SIZE;
}
