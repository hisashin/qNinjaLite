#ifndef _NINJALAMP_CORE_H_
#define _NINJALAMP_CORE_H_

#include "Arduino.h"
#include "ADC.h"
#include <PID_v1.h>

#define TARGET_TEMP 63 /* Typical temp for LAMP */

#define THERMISTOR_LOW_SIDE 1
#define THERMISTOR_HIGH_SIDE 2

#define MAX_OUTPUT_THRESHOLD 5

struct ThermistorRange {
  double tempLowerLimit;
  int bConst;
  double voltageLimit;
};

/* Thermistor config */
struct Thermistor {
  // Thermistor config
  int bConstRangeCount;
  ThermistorRange *bConstRanges;
  double r0;
  double baseTemp;
  
  // Thermistor & resistor circuit
  int place;
  bool useSwitching;
  double r;
  
  // Resistor switching (for well)
  double rLow;
  double rHigh;
  double switchingTemp;
  int switchingPin;
};

class NinjaLAMPCore {
  public:
    NinjaLAMPCore(Thermistor *wellThermistorConf, Thermistor *airThermistorConf, 
      ADCCustom *adc, double wellKP, double wellKI, double wellKD, int heaterPWM);
    // Called from Arduino's setup & loop functions
    void setup();
    void enableSampleTempSimulation (double heatResistanceRatio, double sampleHeatCapacity);
    void NinjaLAMPCore::disableSampleTempSimulation ();
    void loop();
    // Called by interfaces
    void start(double temp);
    void stop();
  private:
    Thermistor *wellThermistor;
    Thermistor *airThermistor;
    ADCCustom *adc;
    bool started;
    PID *pid;
    int heaterPWMPin;
    double wellTemp;
    double airTemp;
    double estimatedSampleTemp;
    void controlTemp();
    void setupPID();
    double readWellTemp ();
    double readAirTemp ();
    double bConstantForVoltage (int rangeCount, ThermistorRange *ranges, double voltageRatio);
    void calcVoltageLimits (Thermistor *thermistor);
    void switchWellR (double temp);
    double voltageToTemp (double voltageRatio, float resistance, 
      float b_constant, float r0, double baseTemp);
    double tempToVoltageRatio (double tempCelsius, double resistance, 
      double bConst, double r0, double baseTemp);
    double averageTemp ();
    double estimatedSampleTemp;
};

#endif _NINJALAMP_CORE_H_
