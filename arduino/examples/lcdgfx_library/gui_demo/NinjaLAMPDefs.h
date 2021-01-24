/**
 * States drive the main loop.
 */
#define STATE_TOP_MENU 1
#define STATE_RUN_MENU 2
#define STATE_SETUP_MENU 3
#define STATE_SETUP_CYCLE 4
#define STATE_RUN_CYCLE 5

/**
 * We currently have a maximim of 3 items in a menu that are not cycles.
 * If you add more then revise the value of MAX_MENU_CYCLES so that it
 * doesn't exceed MAX_MENU_ITEMS
 */
#define MAX_MENU_ITEMS 40
#define MAX_MENU_CYCLES 37

/**
 * EEPROM
 * The data fields start from the end to allow the cycle array to grow.
 * The cycles array start from zero and grow
 * Make sure that the maximum numbwer of cycles do not overlap the data fields.
 */
#define EEPROM_ORIENTATION_LEFT_HANDED_ADDR 255
#define EEPROM_CYCLES_COUNT_ADDR 254
#define EEPROM_CYCLES_MAX_COUNT_ADDR 253
#define EEPROM_CYCLES_START_ADDR 0
#define EEPROM_CYCLES_LEN 4
