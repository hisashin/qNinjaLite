<img src="https://raw.githubusercontent.com/hisashin/NinjaLAMP/master/images/versions.png" alt="versions" width="600">

The configuration for each version can be found in HardwareConf.cpp and HardwareConf.h.
HardwareConf class has two construction methods for two models:

* ver0.9 (initFirstModel)
* ver1.0 (initLCDModel)

Pins, thermistors, PID constants and some model-specific properties for NinjaLAMP's basic functionalities are defined in HardwareConf::initFirstModel() function.

## ver0.9

ver0.9 has no dedicated user interfaces. It sends values every 250　milliseconds by serial communication.
The values can be plotted by the serial plotter of Arduino IDE. This process is implemented in the loop function.

## ver1.0

In April, ver0.9 was released just after a week of rapid prototyping by @hisashin and @maripo. Now @hisashin upgraded its PCB and 3d model  to ver1.0 with widely used [Seeeduino Xiao](https://wiki.seeedstudio.com/Seeeduino-XIAO/), [I2C SSD1306 LCD](https://www.aliexpress.com/item/33028828291.html?spm=a2g0s.9042311.0.0.26274c4dPiM0Ki), [I2C 2kb EEPROM](https://www.digikey.com/en/products/detail/stmicroelectronics/M24C02-RMN6TP/2038677) and 3 tact switches.
- [Hardware updrade is done (2020 Nov 10)](https://www.facebook.com/hisakawa/posts/10158727487954481)
- [New Package (2020 Nov 6)](https://www.facebook.com/hisakawa/posts/10158717794254481)
- [New PCB (2020 Oct 28)](https://www.facebook.com/hisakawa/posts/10158696375544481)
- [KiCad](https://github.com/hisashin/NinjaLAMP/tree/master/kicad/NinjaLAMP)
- [3d model](https://gallery.autodesk.com/projects/149287/ninjalamp)

NinjaLAMP will get following benefits from this upgrade.

- **Standalone** : No need to be connected to PC anymore. LCD displays all informations and we can choose menu by tapping switches as up/down/ok buttons.
- **Programmable** : We can configure custom temperature profiles with switches and store them in EEPROM.
- **Manufacture Friendly** : PCB supports [Seeed PCBA](https://www.seeedstudio.com/prototype-pcb-assembly.html) service and here is their quotation. All electric parts are included and soldered except $1.66 LCD and $0.46 thermistor. With $10 heater and $10 tube holder, NinjaLAMP can be manufactured less than $40. That's great advantage for screening mass poplulation.

<img src="https://github.com/hisashin/NinjaLAMP/blob/master/images/seeed_quotation.jpg" alt="seeed_quotation" width="300">

### Pin assignment

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

---

**For Valerio**

## How to Setup IDE and test ver1.0 for the first time

* Connect NinjaLAMP to your computer with USB TypeC cable. No need to connect 12V AC adapter at first.
* NinjaLAMP ver1.0 uses [Seeeduino XIAO](https://wiki.seeedstudio.com/Seeeduino-XIAO/)(not AVR but SAMD21). Follow their **[Software Step 1 to 5](https://wiki.seeedstudio.com/Seeeduino-XIAO/#software)**
* Upload **"File > Examples > 01.Basics > Blink"** to NinjaLAMP. Orange LED next to USB connector will start blinking.
* Go to **Sketch-> Include Library-> Manage Libraries**, search "ssd1306" and install Alexey Dynda's **[ssd1306 library](https://github.com/lexus2k/ssd1306)**.

<img src="https://raw.githubusercontent.com/hisashin/NinjaLAMP/master/images/ssd1306.png" alt="ssd1306" width="500">

* Restart Arduino IDE
* Upload **"File > Examples > ssd1306 > demos > ssd1306_demo"** to NinjaLAMP. I2C LCD displays [demo](https://github.com/lexus2k/ssd1306/blob/master/examples/demos/ssd1306_demo/ssd1306_demo.ino).
* Clone this NinjaLAMP respository to somewhere
* Upload [NinjaLAMP/arduino/examples/xiao_example_gpio](https://github.com/hisashin/NinjaLAMP/tree/master/arduino/examples/xiao_example_gpio)
* Click magnifying glass icon at the top-right corner and set rate to "9600 baud"
* Tap switches to change values. If adapter is connected, tube holder will be heaten. Be careful.

<img src="https://raw.githubusercontent.com/hisashin/NinjaLAMP/master/images/xiao_example_gpio.png" alt="ssd1306" width="500">

* Upload [NinjaLAMP/arduino/examples/xiao_example_eeprom](https://github.com/hisashin/NinjaLAMP/tree/master/arduino/examples/xiao_example_eeprom)
* Click magnifying glass icon at the top-right corner and set rate to "9600 baud"
* Program writes 0x32(=50) at address 0 of EEPROM on board with writeI2CByte(). Then readI2CByte() read the value of the same address.

<img src="https://raw.githubusercontent.com/hisashin/NinjaLAMP/master/images/xiao_example_eeprom.png" alt="ssd1306" width="500">

* Change (address, value) from (0, 0x32) to another. You can confirm its's saved correctly.

