#include "ADCArduino.h"
#include <NinjaLAMPCore.h>
#include "HardwareConf.h"

/*
  How to customize source before uploading
  https://github.com/hisashin/NinjaLAMP/wiki/How-to-customize-source-before-uploading
*/

/* Target temperature (Celsius) */
#define TARGET_TEMP 63

// NinjaLAMPCore *core = HardwareConf::initFirstModel();
NinjaLAMPCore *core = HardwareConf::initLCDModel();

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
  { 35.0, 20 * 1000 },
  { 45.0, 15 * 1000 },
  { 55.0, 0 },
};
*/
int stageIndex = 0;



// Use 3 lines below to enable temperature simulation feature
#define ENABLE_SAMPLE_TEMP_SIMULATION
#define HEAT_RESISTANCE_RATIO 24.0
#define TUBE_HEAT_CAPACITY 8.0

bool isFinished = false;

void setup() {
  Serial.begin(9600);
  core->setup();
#ifdef ENABLE_SAMPLE_TEMP_SIMULATION
  /*
    How to use simulated sample temperature
    https://github.com/hisashin/NinjaLAMP/wiki/How-to-use-simulated-sample-temperature
  */
  core->enableSampleTempSimulation(HEAT_RESISTANCE_RATIO, TUBE_HEAT_CAPACITY);
#endif /* ENABLE_SAMPLE_TEMP_SIMULATION */
  delay(100);
  core->start(stages[stageIndex].targetTemp);
  delay(100);
}

void loop() {
  core->loop();
  if (Serial.available()) {
    double newTarget =  Serial.parseFloat();
    if (newTarget > 0) {
      core->setTargetTemp(newTarget);
    }
  }
  Serial.print(core->getAirTemp());
  Serial.print("\t");
  Serial.print(core->getWellTemp());
#ifdef ENABLE_SAMPLE_TEMP_SIMULATION
  Serial.print("\t");
  Serial.print(core->getEstimatedSampleTemp());
  Serial.print("\t");
  Serial.print(core->getTempSetpoint());
  Serial.print("\t");
  Serial.print(core->getTargetTemp());
#endif /* ENABLE_SAMPLE_TEMP_SIMULATION */
  Serial.println("");
  if (!isFinished && stages[stageIndex].duration > 0 && 
    stages[stageIndex].duration < core->getStageElapsedTime()) {
    stageIndex += 1;
    if (stageIndex >= stagesCount) {
      // Last stage
      isFinished = true;
      core->stop();
    } else {
      // Next stage
      core->setTargetTemp(stages[stageIndex].targetTemp);
    }
  }
}
