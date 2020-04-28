# NinjaLAMP Arduino

## Customization

### Pinouts

To customize for your hardware, modify lines below (if needed):

NinjaLAMP_Arduino/arduino/NinjaLAMP_Arduino/NinjaLAMP_Arduino.ino lines 5-7

```
const int WELL_HEATER_PWM = 15;
const int WELL_THERMISTOR_AIN = A0;
const int AIR_THERMISTOR_AIN = A1;
```

### Thermistors & resistors

TODO

Resistors

```
/* Well */
struct ThermistorRange wellThermistorRanges[3] = {
  { 0.0, 4250, 0.0, }, /* 4250 for 0-50 deg */
  { 50.0, 4311, 0.0, }, /* 4311 for 50-85 deg */
  { 85.0, 4334, 0.0, } /* 4334 for 85-100 deg */
};
Thermistor wellThermistor = { 
  .bConstRangeCount = 3,
  .bConstRanges = wellThermistorRanges, 
  .r0 = 100.0,
  .baseTemp = 25.0,
  .place = THERMISTOR_LOW_SIDE,
  .useSwitching = false,
  .r = 47.0
};
/* Air */
struct ThermistorRange airThermistorRanges[3] = {
  { 0.0, 4250, 0.0, }, /* 4250 for 0-50 deg */
  { 50.0, 4311, 0.0, }, /* 4311 for 50-85 deg */
  { 85.0, 4334, 0.0, } /* 4334 for 85-100 deg */
};
Thermistor airThermistor = { 
  .bConstRangeCount = 3,
  .bConstRanges = airThermistorRanges, 
  .r0 = 100.0,
  .baseTemp = 25.0,
  .place = THERMISTOR_LOW_SIDE,
  .useSwitching = false,
  .r = 47.0
};
```

PID constants

```
#define WELL_KP (0.11)
#define WELL_KI (0.5)
#define WELL_KD (2.0)
```