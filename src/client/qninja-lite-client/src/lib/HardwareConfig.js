const HardwareConfig = {
    "model": "Ninja-qPCR Development",
    "board_version": "0.1",
    "wells": {
      "count":8,
      "layout": [
        [0,1,2,3,4,5,6,7]
      ],
      "names":[
        "A1",
        "A2",
        "A3",
        "A4",
        "A5",
        "A6",
        "A7",
        "A8"
      ]
    },
    "temperature_measurement":
    [
      {"label":"Plate", "path": "plate.main"},
      {"label":"Block", "path": "plate.block"},
      {"label":"Air", "path": "plate.air"},
      {"label":"Lid", "path": "plate.main"},
      {"label":"Extra A", "path": "extra.0"},
      {"label":"Extra B", "path": "extra.1"}
    ],
    "channels": {
      "count":1
    },
    "hardware_conf":"batch15_hardware_conf"
  }
  module.exports = HardwareConfig;