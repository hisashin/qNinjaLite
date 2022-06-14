from adc_ADS1219IPWR import ADS1219
from led_driver_TLC5929 import TLC5929
from machine import Pin, PWM, SoftI2C, SoftSPI
import time
print("pcr.py")
print("calling test_adc.start()")
adc_device_address = 64
i2c = SoftI2C(scl = Pin(18, Pin.OUT, Pin.PULL_UP), sda = Pin(5, Pin.OUT, Pin.PULL_UP), freq=80000)
adc = ADS1219(i2c, adc_device_address, Pin(17, Pin.IN, Pin.PULL_UP))
adc.start()

# S0=19, S1=21, S2=22, S3=23
mux_s0 = Pin(19, Pin.OUT)
mux_s1 = Pin(21, Pin.OUT)
mux_s2 = Pin(22, Pin.OUT)
mux_s3 = Pin(23, Pin.OUT)
therm_switch = Pin(27, Pin.OUT)
print("therm_switch")
therm_switch.value(0)

selected_well = 0
brightness = 0x7F
well_count = 2
led_channels = [15, 14, 13, 12, 8, 9, 10, 11, 15, 14, 13, 12, 8, 9, 10, 11]
mux_channels = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]
adc.select_single_end_channel(1)

pwm = PWM(Pin(15,Pin.OUT), freq=500)

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
def select_mux (ch): 
    val0 = 0x01 & (ch >> 0)
    val1 = 0x01 & (ch >> 1)
    val2 = 0x01 & (ch >> 2)
    val3 = 0x01 & (ch >> 3)
    print([val0, val1, val2, val3])
    mux_s0.value(val0)
    mux_s1.value(val1)
    mux_s2.value(val2)
    mux_s3.value(val3)
duty = 1023
print ("PCR 2022/04/26 13:34")
while True:
    try:
        time.sleep(0.2)
        adc.select_diff_channels() # Optics
        # adc.select_single_end_channel(1) # Thermistors
        time.sleep(0.1)
        # led.select_led(led_channels[0])
        led.select_led(led_channels[selected_well])
        # led.select_all()
        # led.set_brightness(brightness)
        # select_mux(mux_channels[0])
        select_mux(mux_channels[selected_well])
        # led.on()
        time.sleep(0.3)
        print(adc.read_conversion_data())
        # led.off()
        time.sleep(0.3)
        print(adc.read_conversion_data())
        selected_well = (selected_well + 1) % well_count
        pwm.duty(255)
        duty -= 256
        if (duty < 0):
            duty = 1023
    except Exception as e: 
        print(e)
    