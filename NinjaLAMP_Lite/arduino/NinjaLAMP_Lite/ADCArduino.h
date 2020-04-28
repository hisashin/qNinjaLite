#ifndef ___ADC_ARDUINO___
#define ___ADC_ARDUINO___
#include "ADC.h"

class ADCArduino : public ADC {
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
