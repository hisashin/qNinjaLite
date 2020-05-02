#include "ADCArduino.h"
#include "NinjaLAMPCore.h"

/* Target temperature (Celsius) */
#define TARGET_TEMP 63

/* Pinouts */
const int WELL_HEATER_PWM = 15;
const int WELL_THERMISTOR_AIN = A0;
const int AIR_THERMISTOR_AIN = A1;

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

struct Stage {
  double targetTemp; /* Celsius */
  unsigned long duration; /* msec (0 means holding forever) */
};

const int stagesCount = 1;
struct Stage stages[1] = {
  { TARGET_TEMP, 0 }
};

/*
const int stagesCount = 3;
struct Stage stages[3] = {
  { 35, 20 * 1000 },
  { 45, 15 * 1000 },
  { 55, 20 * 1000 },
};
*/

int stageIndex = 0;

/* PID constants (Dependend on wells and heaters */
#define WELL_KP (0.11)
#define WELL_KI (0.5)
#define WELL_KD (2.0)

NinjaLAMPCore core = {
  .wellThermistorConf = &wellThermistor, 
  .airThermistorConf = &airThermistor,
  .adc = &adc,
  .wellKP = WELL_KP,
  .wellKI = WELL_KI,
  .wellKD = WELL_KD,
  .heaterPWM = WELL_HEATER_PWM
  };

// Use 3 lines below to enable temperature simulation feature
#define ENABLE_SAMPLE_TEMP_SIMULATION
#define HEAT_RESISTANCE_RATIO 24.0
#define TUBE_HEAT_CAPACITY 8.0

bool isFinished = false;

void setup() {
  Serial.begin(9600);
  core.setup();
#ifdef ENABLE_SAMPLE_TEMP_SIMULATION
  core.enableSampleTempSimulation(HEAT_RESISTANCE_RATIO, TUBE_HEAT_CAPACITY);
#endif /* ENABLE_SAMPLE_TEMP_SIMULATION */
  delay(100);
  core.start(stages[stageIndex].targetTemp);
  delay(100);
}

void loop() {
  core.loop();
  if (Serial.available()) {
    double newTarget =  Serial.parseFloat();
    if (newTarget > 0) {
      core.setTargetTemp(newTarget);
    }
  }
  Serial.print(core.getAirTemp());
  Serial.print("\t");
  Serial.print(core.getWellTemp());
#ifdef ENABLE_SAMPLE_TEMP_SIMULATION
  Serial.print("\t");
  Serial.print(core.getEstimatedSampleTemp());
  Serial.print("\t");
  Serial.print(core.getTempSetpoint());
  Serial.print("\t");
  Serial.print(core.getTargetTemp());
#endif /* ENABLE_SAMPLE_TEMP_SIMULATION */
  Serial.println("");
  if (!isFinished && stages[stageIndex].duration > 0 && stages[stageIndex].duration < core.getStageElapsedTime()) {
    stageIndex += 1;
    if (stageIndex >= stagesCount) {
      // Last stage
      isFinished = true;
      core.stop();
    } else {
      // Next stage
      core.setTargetTemp(stages[stageIndex].targetTemp);
    }
  }
}
