# NinjaLAMP Arduino

## Customization

To customize for your hardware, modify  NinjaLAMP_Arduino/arduino/NinjaLAMP_Arduino/NinjaLAMP_Arduino.ino

### Target temperature

```
#define TARGET_TEMP 63
```

### Pinouts

```
const int WELL_HEATER_PWM = 15;
const int WELL_THERMISTOR_AIN = A0;
const int AIR_THERMISTOR_AIN = A1;
```

### Thermistors & resistors

Resistors

```
/* Well */
struct ThermistorRange wellThermistorRanges[3] = {
  { 0.0, 4250, 0.0, }, /* 4250 for 0-50 deg */
  { 50.0, 4311, 0.0, }, /* 4311 for 50-85 deg */
  { 85.0, 4334, 0.0, } /* 4334 for 85-100 deg */
};
Thermistor wellThermistor = { 
  .bConstRangeCount = 3, /* Number of B-constant ranges */
  .bConstRanges = wellThermistorRanges, 
  .r0 = 100.0, /* kOhm */
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
  .bConstRanges = airThermistorRanges, 
  .r0 = 100.0, /* kOhm */
  .baseTemp = 25.0,
  .place = THERMISTOR_LOW_SIDE,
  .useSwitching = false,
  .r = 47.0 /* kOhm */
};
```

PID constants

```
#define WELL_KP (0.11)
#define WELL_KI (0.5)
#define WELL_KD (2.0)
```