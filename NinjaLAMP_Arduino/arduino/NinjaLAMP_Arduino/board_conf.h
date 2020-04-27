#ifndef ___BOARD_CONF_NINJAPCR___
#define ___BOARD_CONF_NINJAPCR___

/* Board Config */
#define USE_ESP8266

#define USE_ADC_NAU7802

// Thermistor value is measured by 2 ranges, switched by analog switch
#define THERMISTOR_2_RANGES
#define PIN_THERMISTOR_RANGE_SWITCH 16 /* Analog switch */

#define USE_WIFI /* Use WiFi functionalities */
#define PIN_WIFI_MODE 16 /* Digitai input pin to switch AP (pairing) mode and Normal mode  */
#define VALUE_WIFI_MODE_AP HIGH /* Start with AP (pairing) mode if value of PIN_WIFI_MODE agrees with this.  */

/* Lid */
#define PIN_LID_THERMISTOR_AIN 1 // TODO Use TOUT
#define PIN_LID_PWM 15 // PWM is available
// #define PIN_LID_PWM_ACTIVE_LOW
#define USE_ESP8266

/* Well */
#ifdef  USE_ADC_NAU7802
#define PIN_WELL_INA 12
#endif
#define PIN_WELL_INB 13
#define PIN_WELL_PWM 4
#define PIN_WELL_PWM_ACTIVE_LOW
#define PIN_WELL_HIGH_TEMP 16

/* Suppress frequent switching of relay */
#define SUPPRESS_PELTIER_SWITCHING

#ifdef USE_ADC_NAU7802
#define PIN_WELL_NAU7802_SCL 14
#define PIN_WELL_NAU7802_SDA 2
#define PIN_WELL_NAU7802_RDY 5
#endif /* USE_ADC_NAU7802 */


#endif /* ___BOARD_CONF_NINJAPCR___ */
