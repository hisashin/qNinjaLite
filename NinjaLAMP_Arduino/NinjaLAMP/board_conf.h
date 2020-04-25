#ifndef ___BOARD_CONF_NINJAPCR___
#define ___BOARD_CONF_NINJAPCR___

#define BAUD_RATE 115200
/* Board Config */
#define USE_ESP8266

/* ADC (NAU7802 or MCP3554) */
#define USE_ADC_NAU7802
//#define USE_ADC_MCP3554
#define TEHRMISTORS_NINJAPCR

// Thermistor value is measured by 2 ranges, switched by analog switch
#define THERMISTOR_2_RANGES
#define PIN_THERMISTOR_RANGE_SWITCH 16 /* Analog switch */

#define USE_WIFI /* Use WiFi functionalities */
#define PIN_WIFI_MODE 16 /* Digitai input pin to switch AP (pairing) mode and Normal mode  */
#define VALUE_WIFI_MODE_AP HIGH /* Start with AP (pairing) mode if value of PIN_WIFI_MODE agrees with this.  */
// #define USE_LCD /* Use display */
// #define USE_STATUS_PINS /* Use status LEDs */
// #define DEBUG_DISPLAY

/* Use LCD */

/* LCD Pins (Dummy) */
#define PIN_LCD_RS 6
#define PIN_LCD_ENABLE 7
#define PIN_LCD_D4 8
#define PIN_LCD_D5 5 // TMP
#define PIN_LCD_D6 16
#define PIN_LCD_D7 17
#define PIN_LCD_CONTRAST 5

/* Lid */
#define PIN_LID_THERMISTOR_AIN 1 // TODO Use TOUT
#define PIN_LID_PWM 15 // PWM is available
// #define PIN_LID_PWM_ACTIVE_LOW
#define USE_ESP8266

/* Well */
#ifdef  USE_ADC_NAU7802
#define PIN_WELL_INA 12
#endif
#ifdef USE_ADC_MCP3554
#define PIN_WELL_INA 12
#endif
#define PIN_WELL_INB 13
#define PIN_WELL_PWM 4
#define PIN_WELL_PWM_ACTIVE_LOW
#define PIN_WELL_HIGH_TEMP 16

/* Fan */
#define USE_FAN
#define PIN_FAN 0
#define PIN_FAN_VALUE_ON HIGH
#define PIN_FAN_VALUE_OFF LOW
/* Suppress frequent switching of relay */
#define SUPPRESS_PELTIER_SWITCHING
/*
 * Define on/off logic values according to
 * polarity of transistors for the relay
 */
#define PIN_WELL_VALUE_ON LOW
#define PIN_WELL_VALUE_OFF HIGH

/* SPI */
#define PIN_WELL_MCP3554_DATAOUT 13//MOSI (Not used)
#define PIN_WELL_MCP3554_DATAIN  12//MISO
#define PIN_WELL_MCP3554_SPICLOCK  14//sck
#define PIN_WELL_MCP3554_SLAVESELECT 5//ss

/* SPI */
#ifdef USE_ADC_MCP3554
#define PIN_WELL_MCP3554_DATAOUT 13//MOSI (Not used)
#define PIN_WELL_MCP3554_DATAIN  12//MISO
#define PIN_WELL_MCP3554_SPICLOCK  14//sck
#define PIN_WELL_MCP3554_SLAVESELECT 5//ss
#endif /* USE_ADC_MCP3554 */

#ifdef USE_ADC_NAU7802
#define PIN_WELL_NAU7802_SCL 14
#define PIN_WELL_NAU7802_SDA 2
#define PIN_WELL_NAU7802_RDY 5
#endif /* USE_ADC_NAU7802 */

#define MIN_PELTIER_PWM -1023
#define MAX_PELTIER_PWM 1023

#define MAX_LID_PWM 1023
#define MIN_LID_PWM 0


#endif /* ___BOARD_CONF_NINJAPCR___ */
