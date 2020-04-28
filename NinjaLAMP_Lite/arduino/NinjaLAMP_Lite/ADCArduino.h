#ifndef ___ADC_ARDUINO___
#define ___ADC_ARDUINO___

class ADCArduino : ADC {
  public:
    ADCArduino();
    void initADC() override;
    double getWellADCValue() override;
    double getAirADCValue() override;
};
#endif /* ___ADC_ARDUINO___ */