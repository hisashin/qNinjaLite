![versions](https://raw.githubusercontent.com/hisashin/NinjaLAMP/master/images/versions.png width=500)

## Hardware config

The configuration for each model can be found in HardwareConf.cpp and HardwareConf.h.
HardwareConf class has two construction methods for two models:

* Initial model (initFirstModel)
* Seeeduino XIAO + LCD model (initLCDModel)

Pins, thermistors, PID constants and some model-specific properties for NinjaLAMP's basic functionalities are defined in HardwareConf::initFirstModel() function.

## How to setup Arduino IDE

You need to setup the Arduino IDE by adding some packages for Seeeduino XIAO, NinjaLAMP's core module.
Please follow the tutorial.

* [Seeeduino XIAO - Seeed Wiki](https://wiki.seeedstudio.com/Seeeduino-XIAO/)


## TODO

The first model has no dedicated user interfaces. It sends values every 250　milliseconds by serial communication.
The values can be plotted by the serial plotter of Arduino IDE. This process is implemented in the loop function.

To make it standalone, we need to implement additional program the tact switches and the display.

## Pin assignment

* LED_R : PA02_A0_D0
* LED_G : PA04_A1_D1
* LED_B : PA10_A2_D2
* HEATER : PA11_A3_D3
* SDA : PA08_A4_D4_SDA
* SCL : PA09_A5_D5_SCL
* AIN_AIR : PB08_A6_D6_TX
* TACT2 : PA06_A10_D10_MOSI
* TACT1 : PA05_A9_D9_MISO
* TACT0 : PA07_A8_D8_SCK
* AIN_WELL : PB09_A7_D7_RX
