from adc_NAU7802 import NAU7802
from led_driver_TLC5929 import TLC5929
from machine import Pin, SoftI2C, SoftSPI, SPI
import time
adc_device_address = 64

i2c = SoftI2C(Pin(18, Pin.OUT, Pin.PULL_UP), Pin(5, Pin.OUT, Pin.PULL_UP), freq=80000)
# adc = ADS1219(i2c, adc_device_address, Pin(17, Pin.IN, Pin.PULL_UP))

adc = NAU7802(i2c, None, 42)
print("calling test_adc.start()")
adc.start()

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

# S0=19, S1=21, S2=22, S3=23
mux_s0 = Pin(19, Pin.OUT)
mux_s1 = Pin(21, Pin.OUT)
mux_s2 = Pin(22, Pin.OUT)
mux_s3 = Pin(23, Pin.OUT)
therm_switch = Pin(27, Pin.OUT)
print("therm_switch")
therm_switch.value(0)
well_offset = 12
selected_well = well_offset
well_count = 16
led_channels = [15, 14, 13, 12, 11,10,9,8, 0,1,2,3,4,5,6,7]
mux_channels = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]

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

print ("PCR 2022/05/17 13:15")
param_a = 0.01
param_b = 0.01
param_c = 20 # Repeat
param_d = 20 # Blink interval

for n in range(5):
    time.sleep(0.5)
    adc.read_conversion_data()

num = 0
print("------------------------")
br = 2
brightness = 1 # 0x7F
while True:
    brightness = br - 1
    # print(["brightness", brightness])
    led.set_brightness(brightness)
    time.sleep(param_a)
    adc.select_analog_input_channel(2) # Optics
    # adc.select_analog_input_channel(1) # Thermistors
    time.sleep(param_b)
    # led.select_led(led_channels[0])
    led.select_led(led_channels[selected_well])
    # led.select_all()
    select_mux(mux_channels[selected_well])
    v = []
    voff = []
    vdiff = []
    for x in range(param_c):
        led.on()
        time.sleep_ms(param_d)
        val_on = adc.read_conversion_data()
        v.append(val_on)
        led.off()
        time.sleep_ms(param_d)
        val_off = adc.read_conversion_data()
        voff.append(val_off)
        vdiff.append(val_on - val_off)
    # print(["Well", selected_well, "x", num])
    # print(v)
    # print(voff)
    print(["FLUO", selected_well, brightness, voff, v, vdiff])
    num = num + 1
    if num == 4:
        num = 0
        br *= 2
        if br > 0x80:
            print("------------------------")
            br = 2

    selected_well = num + well_offset
    # selected_well = (selected_well + 1) % well_count
