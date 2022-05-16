# Ninja Lite

![Image](https://raw.githubusercontent.com/hisashin/NinjaLite/master/images/photo/LiteOpen.jpg)


**2022Feb25 : [Ninja](https://github.com/hisashin/Ninja-qPCR) is almost done. We're importing its optical unit to NinjaLite to run qLAMP.**

<img src="https://raw.githubusercontent.com/hisashin/NinjaLite/master/images/versions.png" alt="versions" width="600">

**Most precise GPL v3 Open source [LAMP (Loop-mediated isothermal amplification)](https://en.wikipedia.org/wiki/Loop-mediated_isothermal_amplification) machine based on Arduino**. Less than 0.3 degree celsius accuracy can be expected after calibration.

![Header](https://github.com/hisashin/NinjaLite/blob/master/images/header.png "header")

### References for COVID-19 detection

- [Rapid colorimetric detection of COVID-19 coronavirus using a reverse tran-scriptional loop-mediated isothermal amplification (RT-LAMP) diagnostic plat-form: iLACO](https://www.medrxiv.org/content/10.1101/2020.02.20.20025874v1)
- [Rapid Molecular Detection of SARS-CoV-2 (COVID-19) Virus RNA Using Colorimetric LAMP](https://www.medrxiv.org/content/10.1101/2020.02.20.20025874v1)

### Why NinjaLite?
- **Precise** with thermal simulation model sensing not only tube holder but air
![Simulation](https://raw.githubusercontent.com/hisashin/NinjaLite/master/images/heat_simulation/illustration_s.png)
- **Easy to build** : Only 4 resistors, 1 mosfet, 2 thermistors are indespensable on [circuit](https://github.com/hisashin/NinjaLite/tree/master/eagle).
- **Easy to use** : [Monitor and control](https://github.com/hisashin/NinjaLite/wiki/Run-and-monitor-temperature-graph) via SerialPlotter of Arduino.
- **Easy to custom** : [Calibration manual](https://github.com/hisashin/NinjaLite/wiki/How-to-use-simulated-sample-temperature) allows you to change core parts.

### Steps

- [BOM (Bill of materials)](https://github.com/hisashin/NinjaLite/wiki/BOM,-Bill-of-Materials) to buy
- Solder parts on universal breadboard like bottom photos of [this page](https://github.com/hisashin/NinjaLite/tree/master/eagle) or order PCB.
- 3d print [base](https://github.com/hisashin/NinjaLite/blob/master/3d/4x4_3d_base.stl) and [cover](https://github.com/hisashin/NinjaLite/blob/master/3d/4x4_3d_cover.stl) upside down without support. Use [belt](https://github.com/hisashin/NinjaLite/blob/master/3d/4x4_3d_belt.stl) to hold PCB or glue. DXF for lasercut is [here](https://github.com/hisashin/NinjaLite/tree/master/dxf) ([3d model](https://gallery.autodesk.com/projects/149287/ninjalamp))
- [Set target temperature](https://github.com/hisashin/NinjaLite/wiki/How-to-customize-source-before-uploading)
- [Upload source to Arduino](https://github.com/hisashin/NinjaLite/wiki/How-to-upload-the-software)
- [Run and monitor temperature graph](https://github.com/hisashin/NinjaLite/wiki/Run-and-monitor-temperature-graph)
- Use cover to keep air temperature high and stable while running.

### Advanced

- [How to use simulated sample temperature](https://github.com/hisashin/NinjaLAMP/wiki/How-to-use-simulated-sample-temperature)
- [How to customize source before uploading](https://github.com/hisashin/NinjaLAMP/wiki/How-to-customize-source-before-uploading)
- [Experiments with multiple temperature stages](https://github.com/hisashin/NinjaLAMP/wiki/Experiments-with-multiple-temperature-stages)
- [How to customize the output](https://github.com/hisashin/NinjaLAMP/wiki/Customizing-the-output)
