#include <PID_v1.h>

/* Customization */
#define TARGET_TEMP 63
#define MAX_OUTPUT_THRESHOLD 5

/* Well Thermistor */
#define B_CONST_25_50 4250
#define B_CONST_25_85 4311
#define R_0_WELL 100.0 // R0
#define R_WELL 47 // Counter resistor
#define THERMISTOR_BASE_TEMP 25.0

/* Pinouts */
const int PIN_HEATER_PWM = 15;
const int THERMISTOR_ANALOG_IN = A0;

/* PID constants */
//#define KP (0.1)
#define KP (0.11)
#define KI (0.5)
#define KD (2.0)

#define KELVIN 273.15

double setpoint, input, output;
// https://playground.arduino.cc/Code/PIDLibraryConstructor/
PID pid(&input, &output, &setpoint, KP, KI, KD, DIRECT);

#define INTERVAL_MSEC 250
#define TEMP_BUFF_SIZE 5

double tempBuff[TEMP_BUFF_SIZE];
int tempBuffIndex = 0;
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
  pinMode(PIN_HEATER_PWM, OUTPUT);
  SWITCHING_VOLTAGE_50 = tempToVoltageRatio(50, R_WELL, B_CONST_25_50, R_0_WELL);
  input = readTemp();
  for (int i=0; i<TEMP_BUFF_SIZE; i++) {
    tempBuff[i] = input;
  }
  setpoint = TARGET_TEMP;
  pid.SetMode(MANUAL);
  pid.SetOutputLimits(-0.5, 0.5);
  pid.SetSampleTime(INTERVAL_MSEC);
}

double prevTemp = 0;
void loop() {
  tempBuff[tempBuffIndex] = readTemp();
  tempBuffIndex = (tempBuffIndex + 1) % TEMP_BUFF_SIZE;
  double temp = averageTemp();
  if (temp < TARGET_TEMP - MAX_OUTPUT_THRESHOLD) {
    // Max drive
    analogWrite(PIN_HEATER_PWM, 1023);
    pid.SetMode(MANUAL);
  } else {
    // PID drive
    input = temp;
    pid.SetMode(AUTOMATIC);
    pid.Compute();
    double pwmOutput = (output + 0.5) * 408/*1024*/;
    analogWrite(PIN_HEATER_PWM, (int)pwmOutput);
  }
  Serial.println(temp);
  delay(INTERVAL_MSEC);
  prevTemp = temp;
}

double readTemp () {

  double voltageRatio = 1.0 - analogRead(THERMISTOR_ANALOG_IN) / 1024.0;
  float bConstant;
  if (voltageRatio > SWITCHING_VOLTAGE_50)
    bConstant = B_CONST_25_50;
  else
    bConstant = B_CONST_25_85;
  return voltageToTemp(voltageRatio, R_WELL, bConstant, R_0_WELL);
}
