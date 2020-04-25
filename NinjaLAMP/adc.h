#ifndef ___ADC_H___
#define ___ADC_H___

#include <stdint.h>

uint8_t initADC ();

enum HardwareStatus {
    HARD_NO_ERROR=0,
    HARD_ERROR_ADC=1,
    HARD_ERROR_LID_DANGEROUS_TEMP=2,
    HARD_ERROR_WELL_DANGEROUS_TEMP=3,
    HARD_ERROR_LID_THERMISTOR_DISCONNECTED=4,
    HARD_ERROR_LID_THERMISTOR_SHORT=5,
    HARD_ERROR_WELL_THERMISTOR_DISCONNECTED=6,
    HARD_ERROR_WELL_THERMISTOR_SHORT=7,
    HARD_ERROR_LID_NOT_REFLECTED=8,
    HARD_ERROR_WELL_NOT_REFLECTED=9,
    HARD_ERROR_WELL_REVERSE=10
};


// Returns (ADC value)/(ADC resolution)
HardwareStatus getWellADCValue (float *val);
HardwareStatus getLidADCValue (float *val);

#endif /* ___ADC_H___ */
