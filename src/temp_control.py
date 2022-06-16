from adc_ADS1219IPWR import ADS1219
from machine import Pin, PWM, SoftI2C
import math
import time
import pid
from pid import PIDRange, PID
from thermistor import Thermistor
from sample_temp_simulation import TempSimulation

print("pcr.py")
print("calling test_adc.start()")
adc_device_address = 64
scl = Pin(18, Pin.OUT, Pin.PULL_UP)
sda = Pin(5, Pin.OUT, Pin.PULL_UP)
i2c = SoftI2C(scl, sda, freq=80000)
adc = ADS1219(i2c, adc_device_address, Pin(17, Pin.IN, Pin.PULL_UP))
adc.start()

# S0=19, S1=21, S2=22, S3=23
mux_s0 = Pin(19, Pin.OUT)
mux_s1 = Pin(21, Pin.OUT)
mux_s2 = Pin(22, Pin.OUT)
mux_s3 = Pin(23, Pin.OUT)
well_heater_pin = Pin(26, Pin.OUT)
well_heater_pin.value(0)
well_heater = PWM(well_heater_pin)

counter_r = 47
thermistor_ali = Thermistor(counter_r, 3950, 100, 25)
thermistor_nx = Thermistor(counter_r, 4311, 100, 25)
thermistor_aki = Thermistor(counter_r, 4250, 100, 25)
thermistor_none = Thermistor(counter_r, 4250, 100, 25)
# well, air, lid, ext1, ext2, ext3
thermistors = [thermistor_ali, thermistor_aki, thermistor_none, thermistor_nx, thermistor_none, thermistor_nx ]

# High/Low temp modes
therm_switch = Pin(27, Pin.OUT)

print("therm_switch")
therm_switch.value(1)

selected_well = 0
well_count = 8
adc.select_single_end_channel(1)

sim = TempSimulation(temp_air=25, temp_well=25)

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
print("sync...")
adc.sync()
print("sync ok")
param_a = 0.35
param_b = 0.35
param_c = 15
param_d = 4

mux_ch_well = 5
mux_ch_lid = 2
mux_ch_air = 3

reg = 0b00000000
started = False
KELVIN = 273.15

resistance = 47
r0 = 100
baseTemp = 25
targetTemp = 100

range_low = PIDRange(kp=0.40, ki=0.035, kd=0.005, min_value=None, max_value=55)
range_mid = PIDRange(kp=0.40, ki=0.035, kd=0.005, min_value=55, max_value=80)
range_high = PIDRange(kp=0.4, ki=0.035, kd=0.005, min_value=80, max_value=None)
ranges = [range_low, range_mid, range_high]
pid = PID(ranges)
interval = 0.5
pid.set_interval(interval)
pid.set_output_range(0, 1.0)
TARGET_TEMP = 84
pid.set_setpoint(TARGET_TEMP)
time_zero = time.ticks_ms()

while True:
    select_mux(mux_ch_well)
    temp_well = thermistors[mux_ch_well].to_temp(adc.read_conversion_data())
    select_mux(mux_ch_air)
    temp_air = thermistors[mux_ch_air].to_temp(adc.read_conversion_data())
    sim.update(temp_air=temp_air, temp_well=temp_well)
    temp_sample = sim.simulate(interval=interval)
    # print("%dch %.2fV, %.2fC" % (mux_ch, v*3.3, temp))
    pid.set_value(temp_well)
    
    output = pid.get_output()
    duty = int(1023.0 * output)
    timestamp = time.ticks_ms() - time_zero
    # print("W=%.2f,A=%.2f,S=%.2f,%d,%.3f,%.3f,%.3f" % (temp_well,temp_air,temp_sample,duty,pid.p, pid.i, pid.d))
    print("%d,%.2f,%.2f" % (timestamp,temp_well,temp_air)) # Print timestamp
    well_heater.duty(duty)
    
    time.sleep(interval)
