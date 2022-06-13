#ifndef _HARDWARE_CONF_
#define _HARDWARE_CONF_

#include <NinjaLAMPCore.h>

class HardwareConf {
  public:
    /*
      First model's hardware configration
      - 4x4 well
      - Well thermistor NXFT15WF104FA2B100
      - Air thermistor NXFT15WF104FA2B100
    */
    static NinjaLAMPCore *initFirstModel ();
    
    /*
      LCD model
      - 8x2 well
      - Well thermistor NXFT15WF104FA2B100
      - Air thermistor NTCG164KF104FT1S
    */
    static NinjaLAMPCore *initLCDModel ();
  private:
};

#endif
