
import math

KELVIN = 273.15
class Thermistor:
    def __init__(self, b_constant, r0, base_temp):
        self.b_constant = b_constant
        self.r0 = r0
        self.base_temp = base_temp
    def to_temp(self, voltage_ratio, resistance):
        thermistor_r = resistance * voltage_ratio / (1.0 - voltage_ratio)
        return (1 / ((math.log(thermistor_r / self.r0) / self.b_constant) + (1.0 / (self.base_temp + KELVIN))))  - KELVIN
