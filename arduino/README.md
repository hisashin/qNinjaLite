<img src="https://raw.githubusercontent.com/hisashin/NinjaLAMP/master/images/versions.png" alt="versions" width="600">

---
**This section is for all ver1.0 user**

## How to Setup

* Connect NinjaLAMP to your computer with USB TypeC cable. No need to connect 12V AC adapter at first.
* NinjaLAMP ver1.0 uses [Seeeduino XIAO](https://wiki.seeedstudio.com/Seeeduino-XIAO/)(not AVR but SAMD21). Follow their **[Software Step 1 to 5](https://wiki.seeedstudio.com/Seeeduino-XIAO/#software)**
* Upload **"File > Examples > 01.Basics > Blink"** to NinjaLAMP. Orange LED next to USB connector will start blinking.
* Go to **Sketch-> Include Library-> Manage Libraries**, search "ssd1306" and install Alexey Dynda's **[ssd1306 library](https://github.com/lexus2k/ssd1306)**.

<img src="https://raw.githubusercontent.com/hisashin/NinjaLAMP/master/images/ssd1306.png" alt="ssd1306" width="500">

* Restart Arduino IDE
* Upload **"File > Examples > ssd1306 > demos > ssd1306_demo"** to NinjaLAMP. I2C LCD displays [demo](https://github.com/lexus2k/ssd1306/blob/master/examples/demos/ssd1306_demo/ssd1306_demo.ino).
* Clone this NinjaLAMP respository to somewhere
* Upload [NinjaLAMP/arduino/xiao_example_gpio](https://github.com/hisashin/NinjaLAMP/tree/master/arduino/xiao_example_gpio)
* Click magnifying glass icon at the top-right corner and set rate to "9600 baud"
* Tap switches to change values. If adapter is connected, tube holders will be heaten. Be careful.

<img src="https://raw.githubusercontent.com/hisashin/NinjaLAMP/master/images/xiao_example_gpio.png" alt="ssd1306" width="500">

---
**This section is for Valerio**

Not yet

---
**Memo**

The configuration for each version can be found in HardwareConf.cpp and HardwareConf.h.
HardwareConf class has two construction methods for two models:

* ver0.9 (initFirstModel)
* ver1.0 (initLCDModel)

Pins, thermistors, PID constants and some model-specific properties for NinjaLAMP's basic functionalities are defined in HardwareConf::initFirstModel() function.

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
