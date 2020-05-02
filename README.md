# NinjaLAMP

GPL v3 Open source [LAMP (Loop-mediated isothermal amplification)](https://en.wikipedia.org/wiki/Loop-mediated_isothermal_amplification) machine. Anyone can make their own LAMP machine by adding any kinds of aluminium tube holder, heater, MOSFET and cheap resistance to Arduino. Calibration manual will be shared soon. accuracy is less than 0.3 degree celsius after calibration.

### References

- [Rapid colorimetric detection of COVID-19 coronavirus using a reverse tran-scriptional loop-mediated isothermal amplification (RT-LAMP) diagnostic plat-form: iLACO](https://www.medrxiv.org/content/10.1101/2020.02.20.20025874v1)
- [Rapid colorimetric detection of COVID-19 coronavirus using a reverse tran-scriptional loop-mediated isothermal amplification (RT-LAMP) diagnostic plat-form: iLACO](https://www.medrxiv.org/content/10.1101/2020.02.20.20025874v1)
](https://www.medrxiv.org/content/10.1101/2020.02.26.20028373v1?fbclid=IwAR3H5GrvC2Q1XY7ty38PRCX5mh1jTq4wlv4akUhHNMqptYJHYWhMloOea0Q)

### Why NinjaLAMP?
- Precise temperature control with thermal simulation model sensing not only tube holder but air
- Support different kinds of core parts guiding how to choose and calibrate to minimize error. Look "Advanced" below.
- Easy to build. Only 4 resistors, 1 mosfet, 2 thermistors on [circuit](https://github.com/hisashin/NinjaLAMP/tree/master/NinjaLAMP_Arduino/eagle).

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

![Top](https://github.com/hisashin/NinjaLAMP/blob/master/images/top.jpg "top")
![Bottom](https://github.com/hisashin/NinjaLAMP/blob/master/images/bottom.jpg "bottom")
![Graph](https://raw.githubusercontent.com/hisashin/NinjaLAMP/master/images/graph.png "graph")
