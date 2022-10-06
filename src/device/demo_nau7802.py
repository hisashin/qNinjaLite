from adc_NAU7802 import NAU7802
from machine import Pin, SoftI2C, SoftSPI, SPI
import time

#    def __init__(self, i2c, drdy, device_address):
# read_conversion_data
i2c = SoftI2C(Pin(18, Pin.OUT, Pin.PULL_UP), Pin(5, Pin.OUT, Pin.PULL_UP), freq=80000)
print("Starting NAU7802 demo. Scan.")
print(i2c.scan())
adc = NAU7802(i2c, None, 42)
time.sleep(1)
adc.start()
time.sleep(1)
print(["REG_0x1F(REV)",adc.read_reg(0x1F)])
while True:
    time.sleep(1)
    adc.select_analog_input_channel(1)
    time.sleep(1)
    print(adc.read_conversion_data())
    time.sleep(1)
    adc.select_analog_input_channel(2)
    time.sleep(1)
    print(adc.read_conversion_data())