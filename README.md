Open source [LAMP (Loop-mediated isothermal amplification)](https://en.wikipedia.org/wiki/Loop-mediated_isothermal_amplification) machine.

Reference : [Rapid colorimetric detection of COVID-19 coronavirus using a reverse tran-scriptional loop-mediated isothermal amplification (RT-LAMP) diagnostic plat-form: iLACO](https://www.medrxiv.org/content/10.1101/2020.02.20.20025874v1)

There are 3 versions.

NinjaLAMP for Arduino (Done) : Just [source codes](https://github.com/hisashin/NinjaLAMP/tree/master/NinjaLAMP_Arduino/NinjaLAMP) for Arduino. Anyone can make their own LAMP machine by adding any kinds of aluminium tube holder, heater, MOSFET and cheap resistance to Arduino. Calibration manual will be shared. accuracy is less than 0.3 degree celsius after calibration.

NinjaLAMP Lite (Developing) : Tube holder, lid heater and PCB of NinjaPCR are packed like [this](https://gallery.autodesk.com/projects/149287/ninjalamp-lite) WiFi supported and its accuracy is less than 0.1 degree celsius. it has 2 minor versions (4x4 tubes and 2x8 tubes). I will make 4x4 tomorrow. 177x126x51mm

NinjaLAMP (Not yet) : After we make [Ninja qPCR](https://github.com/hisashin/Ninja-qPCR), LED+Photodiode array unit will be imported to Lite 2x8 so that we can get results digitally.
