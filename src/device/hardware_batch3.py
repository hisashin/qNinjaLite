#from adc_ADS1219IPWR import ADS1219
from adc_NAU7802 import NAU7802
from machine import Pin, PWM, SoftI2C, SoftSPI, SPI, Timer
from scheduler import Scheduler
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
resistance = 47
r0 = 100
baseTemp = 25
targetTemp = 100
counter_r = 47
temp_switch_val = 0
thermistor_ali = Thermistor(counter_r, 3950, 100, 25)
thermistor_nx = Thermistor(counter_r, 4311, 100, 25)
thermistor_aki = Thermistor(counter_r, 4250, 100, 25)
thermistor_none = Thermistor(counter_r, 4250, 100, 25)
well_heater_pin = Pin(25, Pin.OUT)
well_heater_pin.value(0)
well_heater = PWM(well_heater_pin)
well_heater.duty(0)
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
print ("PCR 2022/05/25 11:07")

param_a = 0.35
param_b = 0.35
param_c = 15
param_d = 4


CONTROL_INTERVAL_MSEC = 1000
CONTROL_INTERVAL_SEC = CONTROL_INTERVAL_MSEC/1000.0
interval = CONTROL_INTERVAL_SEC
TARGET_TEMP = 94
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
    def __init__(self, scheduler):
        self.schedule = scheduler.add_schedule()
        self.is_measuring = False
        self.channel_index = 0
        self.well_index = 0
    def measure_all(self, callback):
        print("measure_all")
        if self.is_measuring:
            return False # Rejected (busy)
        self.callback = callback
        self.channel_index = 0
        self.well_index = 0
        self.is_measuring = True
        self.measurement = []
        for i in range(CHANNEL_COUNT):
            self.measurement.append([0] * WELL_COUNT)
        self.schedule.init_timer(200, Timer.PERIODIC, self.measure_next)
        return True # Accepted
    def measure_next (self):
        led.select_led(led_channels[self.well_index])
        led.set_brightness(brightness)
        select_mux(mux_channels[self.well_index])
        time.sleep(0.01)
        adc.select_analog_input_channel(2) 
        time.sleep(0.01)
        led.off()
        time.sleep(0.03)
        voff = adc.read_conversion_data()
        led.on()
        time.sleep(0.03)
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

class TempControl:
    def __init__(self, scheduler):
        self.schedule = scheduler.add_schedule()
        self.temp = DEFAULT_TEMP
        self.target_temp = DEFAULT_TEMP
        self.sim = TempSimulation(temp_air=25, temp_well=25)
        self.temp_units = [
            # MUX index, Thermistor, temp, label
            [0, thermistor_ali, DEFAULT_TEMP, "Well"],
            [1, thermistor_aki, DEFAULT_TEMP, "Air"]
        ]
        range_low = PIDRange(kp=0.40, ki=0.035, kd=0.005, min_value=None, max_value=55)
        range_mid = PIDRange(kp=0.40, ki=0.035, kd=0.005, min_value=55, max_value=80)
        range_high = PIDRange(kp=0.4, ki=0.035, kd=0.005, min_value=80, max_value=None)
        ranges = [range_low, range_mid, range_high]
        pid = PID(ranges)
        pid.set_interval(CONTROL_INTERVAL_SEC)
        pid.set_output_range(0, 1.0)
        self.pid = pid
    def control (self):
        print("control")
        self.termistor_index = 0
        self.schedule.init_timer(200, Timer.PERIODIC, self.measure_next)
        well = self.temp_units[0]
        air = self.temp_units[1]
        self.pid.set_value(well[2])
        output = self.pid.get_output()
        duty = int(256.0 * output)
        # duty = 0
        print("W=%.2f\tA=%.2f\tO=%.2f" % (well[2], air[2], output)) # Print timestamp
        well_heater.duty(duty)
    def measure_next (self):
        adc.select_analog_input_channel(1)
        temp_unit = self.temp_units[self.termistor_index]
        select_mux(temp_unit[0])
        thermistor = temp_unit[1]
        time.sleep(0.05)
        temp_unit[2] = thermistor.to_temp(adc.read_conversion_data())
        print("measure_next", self.termistor_index, temp_unit[2])
            
        if self.termistor_index == 1:
            self.schedule.cancel_timer()
        self.termistor_index += 1
    def set_target_temp(self, temp):
        self.target_temp = temp
        self.pid.set_setpoint(temp)
        print("setTargetTemp", self.target_temp)

count = 0
scheduler = Scheduler()
temp_control = TempControl(scheduler)
temp_control.set_target_temp(TARGET_TEMP)
optics = Optics(scheduler)

def optics_on_measure(values):
    print(values)
def progress ():
    global count
    temp_control.control()
    count += 1
    if count == 10:
        optics.measure_all(optics_on_measure)
        count = 0

control_schedule = scheduler.add_schedule()
control_schedule.init_timer(CONTROL_INTERVAL_MSEC, Timer.PERIODIC, progress)

for n in range(5):
    time.sleep(0.5)
    adc.read_conversion_data()
while True:
    scheduler.loop()
    time.sleep(0.01)
