# Hardware control for batch3 board
# - Temp control
# - Optics unit control

from adc_NAU7802 import NAU7802
from machine import Pin, PWM, SoftI2C, SoftSPI, SPI, Timer
from led_driver_TLC5929 import TLC5929
import time
from pid import PIDRange, PID
from thermistor import Thermistor
from sample_temp_simulation import TempSimulation

adc_device_address = 42
scl = Pin(18, Pin.OUT, Pin.PULL_UP)
sda = Pin(5, Pin.OUT, Pin.PULL_UP)
i2c = SoftI2C(scl, sda, freq=80000)
adc = NAU7802(i2c, None, adc_device_address)
adc.start()
adc.select_conversion_rate(330)

# Thermal control

KELVIN = 273.15
targetTemp = 100
counter_r = 47
temp_switch_val = 0

thermistor_ali = Thermistor(3950, 100, 25)
thermistor_nx = Thermistor(4311, 100, 25)
thermistor_aki = Thermistor(4250, 100, 25)
thermistor_none = Thermistor(4250, 100, 25)

well_heater_pin = Pin(25, Pin.OUT)
well_heater_pin.value(0)
well_heater = PWM(well_heater_pin, duty=0)

therm_switch = Pin(27, Pin.OUT)
therm_switch.value(temp_switch_val)
# well, air, lid, ext1, ext2, ext3
# thermistors = [thermistor_ali, thermistor_aki, thermistor_none, thermistor_nx, thermistor_none, thermistor_nx ]
# High/Low temp modes

# Multiplexer control
mux_s0 = Pin(19, Pin.OUT)
mux_s1 = Pin(21, Pin.OUT)
mux_s2 = Pin(22, Pin.OUT)
mux_s3 = Pin(23, Pin.OUT)

def select_mux (ch): 
    val0 = 0x01 & (ch >> 0)
    val1 = 0x01 & (ch >> 1)
    val2 = 0x01 & (ch >> 2)
    val3 = 0x01 & (ch >> 3)
    mux_s0.value(val0)
    mux_s1.value(val1)
    mux_s2.value(val2)
    mux_s3.value(val3)

param_a = 0.35
param_b = 0.35
param_c = 15
param_d = 4

CONTROL_INTERVAL_MSEC = 500
CONTROL_INTERVAL_SEC = CONTROL_INTERVAL_MSEC/1000.0
CHANNEL_COUNT = 1
WELL_COUNT = 8
DEFAULT_TEMP = 25

# LED Driver
# SCLK=12, MOSI=13, MISO=15, LAT=14
sclk=Pin(12, Pin.OUT)
mosi=Pin(13, Pin.OUT)
blank=Pin(15, Pin.OUT)
latch=Pin(14,Pin.OUT)
sclk.off()
mosi.off()
latch.off()
blank.off()
print("Init SPI...")
spi = SoftSPI(baudrate=10000, polarity=0, phase=0, firstbit=SPI.MSB, sck=sclk, mosi=mosi, miso=Pin(16))
print("Init SPI done")
led = TLC5929(spi, latch, blank)
led_channels = [15, 14, 13, 12, 11,10,9,8, 0,1,2,3,4,5,6,7]
mux_channels = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]
brightness = 0x7F
class Optics:
    def __init__(self, scheduler, measure_interval_ms=200):
        self.schedule = scheduler.add_schedule()
        self.is_measuring = False
        self.channel_index = 0
        self.well_index = 0
        self.measure_interval_ms = measure_interval_ms
    def measure_all(self, callback):
        print("OptAll")
        if self.is_measuring:
            return False # Rejected (busy)
        self.callback = callback
        self.channel_index = 0
        self.well_index = 0
        self.is_measuring = True
        self.measurement = []
        for i in range(CHANNEL_COUNT):
            self.measurement.append([0] * WELL_COUNT)
        self.schedule.init_timer(self.measure_interval_ms, Timer.PERIODIC, self.measure_next)
        return True # Accepted
    def measure_next (self):
        led.select_led(led_channels[self.well_index])
        led.set_brightness(brightness)
        led.off()
        select_mux(mux_channels[self.well_index])
        adc.select_analog_input_channel(2) # Optics channel
        time.sleep(0.02)
        voff = adc.read_conversion_data()
        led.on()
        time.sleep(0.02)
        von = adc.read_conversion_data()
        v = voff - von
        try:
            self.measurement[self.channel_index][self.well_index] = v
            if self.well_index == WELL_COUNT - 1:
                # Next channel & first well
                if self.channel_index == CHANNEL_COUNT - 1:
                    # All done
                    self.is_measuring = False
                    self.schedule.cancel_timer()
                    self.callback(self.measurement)
                self.well_index = 0
                self.channel_index += 1
            else:
                # Next well
                self.well_index += 1
        except:
            print(["Except???", self.channel_index, self.well_index])

# Switch value
RESISTOR_SWITCH_LOW = 0
RESISTOR_SWITCH_HIGH = 1
# Resistor value
RESISTOR_LOW = 47 # kOhm
RESISTOR_HIGH = 10 # kOhm
# Switching temperature
RESISTOR_TEMP_LIMIT_HIGH = 50
RESISTOR_TEMP_LIMIT_LOW = 45

# Default PID ranges

PID_RANGES_DEFAULT = [
    PIDRange(kp=0.073, ki=0.06, kd=0.02, min_value=None, max_value=75), 
    PIDRange(kp=0.11, ki=0.08, kd=0.025, min_value=75, max_value=None)
    ]
class TempUnit:
    def __init__ (self, mux_index, thermistor, label, resistor_switch):
        self.mux_index = mux_index
        self.thermistor = thermistor
        self.label = label
        self.resistor_switch = resistor_switch
        self.temp = 25
        self.resistor = RESISTOR_LOW

class TempControl:
    def __init__(self, scheduler, measure_interval_ms=100, pid_interval_ms=500):
        self.schedule = scheduler.add_schedule()
        self.target_temp = DEFAULT_TEMP
        self.sim = TempSimulation(temp_air=25, temp_well=25)
        self.measure_interval_ms = measure_interval_ms
        self.pid_interval_ms = pid_interval_ms
        self.temp_unit_well = TempUnit(0, thermistor_ali, "Well", RESISTOR_SWITCH_LOW)
        self.temp_unit_air = TempUnit(1, thermistor_aki, "Air", RESISTOR_SWITCH_LOW)
        self.temp_units = [ self.temp_unit_well, self.temp_unit_air ]
        self.config_pid(PID_RANGES_DEFAULT)
    def config_pid(self, ranges):
        print("config_pid")
        pid = PID(ranges)
        pid.set_interval(self.pid_interval_ms/1000.0)
        pid.set_output_range(0, 1.0)
        self.pid = pid
    def set_pid_constants(self, ranges):
        pid_ranges = []
        for range in ranges:
            print("set_pid_constants",range["kp"], range["ki"],range["kd"], range.get("min_value", None), range.get("max_value", None))
            pid_ranges.append(PIDRange(kp=range["kp"], ki=range["ki"], kd=range["kd"], 
                min_value=range.get("min_value", None), max_value=range.get("max_value", None)))
        self.config_pid(pid_ranges)
        

    def get_temp(self):
        return self.temp_unit_well.temp
    def control (self):
        print("TmpCtrl")
        self.termistor_index = 0
        self.schedule.init_timer(150, Timer.PERIODIC, self.measure_next)
        self.pid.set_value(self.temp_unit_well.temp)
        output = self.pid.get_output()
        duty = int(512.0 * output)
        # duty = 0
        # print("W=%.2f\tA=%.2f\tO=%.2f" % (self.temp_unit_well.temp, self.temp_unit_air.temp, output)) # Print timestamp
        well_heater.duty(duty)
    def measure_next (self):
        adc.select_analog_input_channel(1) # Optics channel
        temp_unit = self.temp_units[self.termistor_index]
        therm_switch.value(temp_unit.resistor_switch)
        select_mux(temp_unit.mux_index)
        time.sleep(0.05)
        temp_unit.temp = temp_unit.thermistor.to_temp(adc.read_conversion_data(), temp_unit.resistor)
        if temp_unit.resistor_switch == RESISTOR_SWITCH_LOW and temp_unit.temp > RESISTOR_TEMP_LIMIT_HIGH:
            print("To High Temp Mode")
            temp_unit.resistor_switch = RESISTOR_SWITCH_HIGH
            temp_unit.resistor = RESISTOR_HIGH
        if temp_unit.resistor_switch == RESISTOR_SWITCH_HIGH and temp_unit.temp < RESISTOR_TEMP_LIMIT_LOW:
            print("To Low Temp Mode")
            temp_unit.resistor_switch = RESISTOR_SWITCH_LOW
            temp_unit.resistor = RESISTOR_LOW
            
        if self.termistor_index == 1:
            self.schedule.cancel_timer()
        self.termistor_index += 1
    def set_target_temp(self, temp):
        self.target_temp = temp
        self.pid.set_setpoint(temp)
        print("setTargetTemp", self.target_temp)

def init_hardware():
    well_heater.duty(0) # Heater off.
    adc.select_conversion_rate(330)
    adc.select_analog_input_channel(1)
    print("init_hardware start.")
    for i in range(5):
        adc.read_conversion_data()
        time.sleep(0.1)
    print("init_hardware end.")
    