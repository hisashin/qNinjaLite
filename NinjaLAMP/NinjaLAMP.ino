#include <PID_v1.h>

#define SETPOINT 63
#define THERMISTOR_BASE_TEMP 25.0

#define KELVIN 273.15

// Well Thermistor
#define B_CONST_25_50 4250
#define B_CONST_25_85 4311
#define R_0_WELL 100.0
#define R_WELL 47

#define PIN_LID_PWM 15

//#define KP (0.060)
#define KP (0.055)
#define KI (20)
#define KD (0.05)


double setpoint, input, output;
// https://playground.arduino.cc/Code/PIDLibraryConstructor/
PID pid(&input, &output, &setpoint, KP, KI, KD, DIRECT);

static double SWITCHING_VOLTAGE_50 = 0;

static float INVERSE_BASE_TEMP = 1.0 / (THERMISTOR_BASE_TEMP + KELVIN);
double voltageToTemp (double voltageRatio, float resistance, float b_constant, float r0) {
  double thermistorR = resistance * voltageRatio / (1.0 - voltageRatio);
  return (1 / ((log(thermistorR / r0) / b_constant) + INVERSE_BASE_TEMP))  - KELVIN;
}
double tempToVoltageRatio (double tempCelsius, double resistance, double bConst, double r0) {
  double thermistorR = r0 * exp(bConst * (1 / (tempCelsius + KELVIN) - 1 / (THERMISTOR_BASE_TEMP + KELVIN)));
  return thermistorR / (thermistorR + resistance);
}

#define INTERVAL_MSEC 250
#define TEMP_BUFF_SIZE 5

double tempBuff[TEMP_BUFF_SIZE];
int tempBuffIndex = 0;
double averageTemp () {
  double tempSum = 0;
  for (int i=0; i<TEMP_BUFF_SIZE; i++) {
    tempSum += tempBuff[i];
  }
  return tempSum / TEMP_BUFF_SIZE;
}
void setup() {
  Serial.begin(9600);
  pinMode(PIN_LID_PWM, OUTPUT);
  SWITCHING_VOLTAGE_50 = tempToVoltageRatio(50, R_WELL, B_CONST_25_50, R_0_WELL);
  input = readTemp();
  for (int i=0; i<TEMP_BUFF_SIZE; i++) {
    tempBuff[i] = input;
  }
  setpoint = SETPOINT;
  pid.SetMode(AUTOMATIC);
  pid.SetOutputLimits(0, 1);
  pid.SetSampleTime(INTERVAL_MSEC);
}

void loop() {
  tempBuff[tempBuffIndex] = readTemp();
  tempBuffIndex = (tempBuffIndex + 1) % TEMP_BUFF_SIZE;
  //Serial.print("Temp="); Serial.print(temp);
  double temp = averageTemp();
  input = temp;
  pid.Compute();
  double pwmOutput = output * 1024;
  if (pwmOutput > 1023) {
    pwmOutput = 1023;
  }
  analogWrite(PIN_LID_PWM, (int)pwmOutput);
  Serial.println(temp);
  delay(INTERVAL_MSEC);
}
const int analog0 = A0;

double readTemp () {

  double voltageRatio = 1.0 - analogRead(analog0) / 1024.0;
  float bConstant;
  if (voltageRatio > SWITCHING_VOLTAGE_50)
    bConstant = B_CONST_25_50;
  else
    bConstant = B_CONST_25_85;
  return voltageToTemp(voltageRatio, R_WELL, bConstant, R_0_WELL);
}
