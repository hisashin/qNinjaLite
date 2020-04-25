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
