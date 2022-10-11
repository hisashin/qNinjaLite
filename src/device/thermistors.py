# from adc_ADS1219IPWR import ADS1219
from adc_NAU7802 import NAU7802
from machine import Pin, SoftI2C, PWM
import time
from thermistor import Thermistor
print("pcr.py")
print("calling test_adc.start()")
adc_device_address = 42
scl = Pin(18, Pin.OUT, Pin.PULL_UP)
sda = Pin(5, Pin.OUT, Pin.PULL_UP)
i2c = SoftI2C(scl, sda, freq=80000)
# adc = ADS1219(i2c, adc_device_address, Pin(17, Pin.IN, Pin.PULL_UP))
adc = NAU7802(i2c, None, adc_device_address)
adc.start()

# S0=19, S1=21, S2=22, S3=23
mux_s0 = Pin(19, Pin.OUT)
mux_s1 = Pin(21, Pin.OUT)
mux_s2 = Pin(22, Pin.OUT)
mux_s3 = Pin(23, Pin.OUT)

# High/Low temp modes
therm_switch = Pin(27, Pin.OUT)

selected_well = 0
well_count = 8
# adc.select_single_end_channel(1)

adc.select_analog_input_channel(1)
adc.select_conversion_rate(330)


def select_mux (ch): 
    val0 = 0x01 & (ch >> 0)
    val1 = 0x01 & (ch >> 1)
    val2 = 0x01 & (ch >> 2)
    val3 = 0x01 & (ch >> 3)
    mux_s0.value(val0)
    mux_s1.value(val1)
    mux_s2.value(val2)
    mux_s3.value(val3)
print ("PCR 2022/05/13 13:30")

mux_ch = 0

reg = 0b00000000
started = False

# Switch=High -> 10k
# Switch=Low -> 47k
counter_r = 47
switch_val = 0
therm_switch.value(switch_val)
# switch=0, R=10 / switch=1, R-47

# ALI 3950
# NX 4334
# AKI 4334

# 0ch 2.16V, 27.57C (ALI テープ接着) well
# 1ch 1.85V, 36.01C (NX on board) air ... だんだんあったまる
# 2ch 3.30V, -84.38C (NONE) lid
# 3ch 2.08V, 29.65C (NX空中) ext1 ... airとして良さそう
# 4ch 3.30V, -84.64C (NONE) ext2                                                                                                                               
# 5ch 2.15V, 27.74C (NX 穴あけ接着) ext3

thermistor_ali = Thermistor(3950, 100, 25)
thermistor_nx = Thermistor(4311, 100, 25)
thermistor_aki = Thermistor(4250, 100, 25)
thermistor_none = Thermistor(4250, 100, 25)
# well, air, lid, ext1, ext2, ext3
thermistors = [thermistor_ali, thermistor_aki, thermistor_none, thermistor_nx, thermistor_none, thermistor_nx ]

adc.read_conversion_data()

if True:
    well_heater_pin = Pin(25, Pin.OUT)
    well_heater_pin.value(0)
    well_heater = PWM(well_heater_pin)
    well_heater.duty(182) #1023 all on

time_zero = time.ticks_ms()

while True:
    try:
        select_mux(mux_ch)
        time.sleep(0.125)
        v = adc.read_conversion_data() # (adc.read_conversion_data() + 1) / 2
        temp = thermistors[mux_ch].to_temp(v, counter_r)
        print("%dch %.2fV, %.2fC" % (mux_ch, v * 3.3, temp))
        # print(switch_val, counter_r)
        time.sleep(0.375)

        if mux_ch == 0:
            if switch_val == 0 and temp > 60:
                print("To High Temp Mode")
                counter_r = 10
                switch_val = 1
            if switch_val == 1 and temp < 50:
                print("To Low Temp Mode")
                counter_r = 47
                switch_val = 0
            therm_switch.value(switch_val)
            time.sleep(0.25)
        mux_ch = (mux_ch+1)%2
    except Exception as e: 
        print(e)
    