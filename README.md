# NinjaLAMP

**Most precise GPL v3 Open source [LAMP (Loop-mediated isothermal amplification)](https://en.wikipedia.org/wiki/Loop-mediated_isothermal_amplification) machine based on Arduino**. We share 3dpint/lasercut model but that is just an example. You can make your own LAMP machine by assembling any kinds of aluminium tube holder, heater, MOSFET and cheap resistor to Arduino. PID Calibration manual is [here](https://github.com/hisashin/NinjaLAMP/wiki/How-to-use-simulated-sample-temperature). Less than 0.3 degree celsius accuracy can be expected after calibration.

![Top](https://github.com/hisashin/NinjaLAMP/blob/master/images/top.jpg "top")
![Bottom](https://github.com/hisashin/NinjaLAMP/blob/master/images/bottom.jpg "bottom")
![Graph](https://raw.githubusercontent.com/hisashin/NinjaLAMP/master/images/graph.png "graph")

### References for COVID-19 detection

- [Rapid colorimetric detection of COVID-19 coronavirus using a reverse tran-scriptional loop-mediated isothermal amplification (RT-LAMP) diagnostic plat-form: iLACO](https://www.medrxiv.org/content/10.1101/2020.02.20.20025874v1)
- [Rapid Molecular Detection of SARS-CoV-2 (COVID-19) Virus RNA Using Colorimetric LAMP](https://www.medrxiv.org/content/10.1101/2020.02.20.20025874v1)

### Why NinjaLAMP?
- **Precise** with thermal simulation model sensing not only tube holder but air
![Simulation](https://raw.githubusercontent.com/hisashin/NinjaLAMP/master/images/heat_simulation/illustration_s.png)
- **Easy to custom** by supporting wide range of core parts. Software is also easy to extend.
- **Easy to build** Only 4 resistors, 1 mosfet, 2 thermistors on [circuit](https://github.com/hisashin/NinjaLAMP/tree/master/eagle).

### Steps to make

- Choose [3dprint]() or [lasercut]()
- [BOM (Bill of materials)](https://github.com/hisashin/NinjaLAMP/wiki/%5BArduino%5D-BOM) to buy
- 3d print [base](https://github.com/hisashin/NinjaLAMP/blob/master/NinjaLAMP_Arduino/3d/4x4_3d_base.stl) and [cover](https://github.com/hisashin/NinjaLAMP/blob/master/NinjaLAMP_Arduino/3d/4x4_3d_cover.stl) upside down without support ([3d model](https://gallery.autodesk.com/projects/149287/ninjalamp))
- Set target temperature
- [Upload source to Arduino](https://github.com/hisashin/NinjaLAMP/wiki/%5BArduino%5D-How-to-upload-the-software)
- [Run and monitor temperature graph](https://github.com/hisashin/NinjaLAMP/wiki/Run-and-monitor-temperature-graph)

### Advanced

- How to choose and calibrate different tube holder, heater, thermistor
- [How to use simulated sample temperature](https://github.com/hisashin/NinjaLAMP/wiki/How-to-use-simulated-sample-temperature)
- [How to customize source before uploading](https://github.com/hisashin/NinjaLAMP/wiki/%5BArduino%5D-How-to-customize-source-before-uploading)
- [Experiments with multiple temperature stages](https://github.com/hisashin/NinjaLAMP/wiki/Experiments-with-multiple-temperature-stages)

