# NinjaLAMP

**Most precise GPL v3 Open source [LAMP (Loop-mediated isothermal amplification)](https://en.wikipedia.org/wiki/Loop-mediated_isothermal_amplification) machine based on Arduino**. Less than 0.3 degree celsius accuracy can be expected after calibration.

![Header](https://github.com/hisashin/NinjaLAMP/blob/master/images/header.png "header")

### References for COVID-19 detection

- [Rapid colorimetric detection of COVID-19 coronavirus using a reverse tran-scriptional loop-mediated isothermal amplification (RT-LAMP) diagnostic plat-form: iLACO](https://www.medrxiv.org/content/10.1101/2020.02.20.20025874v1)
- [Rapid Molecular Detection of SARS-CoV-2 (COVID-19) Virus RNA Using Colorimetric LAMP](https://www.medrxiv.org/content/10.1101/2020.02.20.20025874v1)

### Why NinjaLAMP?
- **Precise** with thermal simulation model sensing not only tube holder but air
![Simulation](https://raw.githubusercontent.com/hisashin/NinjaLAMP/master/images/heat_simulation/illustration_s.png)
- **Easy to build** Only 4 resistors, 1 mosfet, 2 thermistors are indespensable on [circuit](https://github.com/hisashin/NinjaLAMP/tree/master/eagle).
- **Easy to use** [Monitor and control](https://github.com/hisashin/NinjaLAMP/wiki/Run-and-monitor-temperature-graph) via SerialPlotter of Arduino.
- **Easy to custom** [Calibration manual](https://github.com/hisashin/NinjaLAMP/wiki/How-to-use-simulated-sample-temperature) allows you to change core parts.

### Steps to make

- [BOM (Bill of materials)](https://github.com/hisashin/NinjaLAMP/wiki/BOM,-Bill-of-Materials) to buy
- 3d print [base](https://github.com/hisashin/NinjaLAMP/blob/master/3d/4x4_3d_base.stl) and [cover](https://github.com/hisashin/NinjaLAMP/blob/master/3d/4x4_3d_cover.stl) upside down without support. Use [belt](https://github.com/hisashin/NinjaLAMP/blob/master/3d/4x4_3d_belt.stl) to hold PCB or glue. ([3d model](https://gallery.autodesk.com/projects/149287/ninjalamp))
- We have [DXF](https://github.com/hisashin/NinjaLAMP/tree/master/dxf) for lasercut too.
- [Set target temperature](https://github.com/hisashin/NinjaLAMP/wiki/How-to-customize-source-before-uploading)
- [Upload source to Arduino](https://github.com/hisashin/NinjaLAMP/wiki/How-to-upload-the-software)
- [Run and monitor temperature graph](https://github.com/hisashin/NinjaLAMP/wiki/Run-and-monitor-temperature-graph)

### Advanced

- How to choose and calibrate different tube holder, heater, thermistor
- [How to use simulated sample temperature](https://github.com/hisashin/NinjaLAMP/wiki/How-to-use-simulated-sample-temperature)
- [How to customize source before uploading](https://github.com/hisashin/NinjaLAMP/wiki/How-to-customize-source-before-uploading)
- [Experiments with multiple temperature stages](https://github.com/hisashin/NinjaLAMP/wiki/Experiments-with-multiple-temperature-stages)
- [How to customize the output](https://github.com/hisashin/NinjaLAMP/wiki/Customizing-the-output)

