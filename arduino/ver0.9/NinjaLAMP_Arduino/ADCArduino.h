#ifndef ___ADC_ARDUINO___
#define ___ADC_ARDUINO___
#include "adc.h"

class ADCArduino : public ADCCustom {
  public:
    ADCArduino(int ainWell, int ainAir);
    void initADC() override;
    double getWellADCValue() override;
    double getAirADCValue() override;
  private:
    int analogInWell;
    int analogInAir;
};
#endif /* ___ADC_ARDUINO___ */
