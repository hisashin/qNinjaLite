#ifndef ___ADC_NAU7802___
#define ___ADC_NAU7802___
#include "adc.h"
// Slave Address (Fixed)
#define NAU7802_DEVICE_ADDRESS  0x2A

// Register Address
#define NAU7802_REG_ADDR_PU_CTRL 0x00 // Initialization
#define NAU7802_REG_ADDR_ADCO_B2 0x12 // ADC value (Followed by next 2 registers)
#define NAU7802_REG_ADDR_CTRL2   0x02 // Calibration
#define NAU7802_REG_ADDR_REVISION  0x1F

#define NAU7802_BIT_RR 0 // Register Reset
#define NAU7802_BIT_PUD 1 // Power up digital circuit
#define NAU7802_BIT_PUA 2 // Power up analog circuit
#define NAU7802_BIT_PUR 3 // Power up ready (Read Only Status)
#define NAU7802_BIT_CS 4 // Cycle start
#define NAU7802_BIT_CR 5 // Cycle ready (Read only Status)
#define NAU7802_BIT_OSCS 6 // System clock source select
#define NAU7802_BIT_AVDDS 7 // AVDD source select

/*
111 = 320SPS
011 = 80SPS
010 = 40SPS
001 = 20SPS
000 = 10SPS
*/
#define NAU7802_RATE_320SPS 0b0111
#define NAU7802_RATE_80SPS 0b0011
#define NAU7802_RATE_40SPS 0b0010
#define NAU7802_RATE_20SPS 0b0001
#define NAU7802_RATE_10SPS 0b0000 //default


#include <stdint.h>


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
/*
HardwareStatus getWellADCValue (float *val);
HardwareStatus getAirADCValue (float *val);
*/

class ADCNAU7802 : public ADC {
  public:
    ADCNAU7802();
    void initADC() override;
    double getWellADCValue() override;
    double getAirADCValue() override;
};

#endif /* ___ADC_NAU7802___ */
