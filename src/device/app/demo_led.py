# LED only
from led_driver_TLC5929 import TLC5929
from machine import Pin, SoftI2C, SoftSPI, SPI
import time
lcd_device_address = 64

print("calling test_adc.start()")
if False:
    # batch 3
    sclk=Pin(12, Pin.OUT)
    mosi=Pin(13, Pin.OUT)
    miso=Pin(16, Pin.IN)
    blank=Pin(15, Pin.OUT)
    latch=Pin(14,Pin.OUT)
if False:
    # batch 4
    sclk=Pin(18, Pin.OUT)
    mosi=Pin(23, Pin.OUT)
    miso=Pin(19, Pin.IN)
    blank=Pin(15, Pin.OUT)
    latch=Pin(14,Pin.OUT)

sclk=Pin(18, Pin.OUT)
mosi=Pin(23, Pin.OUT)
miso=Pin(19, Pin.IN)
blank=Pin(15, Pin.OUT)
latch=Pin(14,Pin.OUT)

sclk.off()
mosi.off()
latch.off()
blank.off()
print("Init SPI...")
spi = SoftSPI(baudrate=10000, polarity=0, phase=0, firstbit=SPI.MSB, sck=sclk, mosi=mosi, miso=miso)
print("Init SPI done")
led = TLC5929(spi, latch, blank)

led_brightness = 0x70
well_count = 8
led_channels = [15,14,13,12,11,10,9,8, 0,1,2,3,4,5,6,7]

i2c = SoftI2C(Pin(18, Pin.OUT, Pin.PULL_UP), Pin(5, Pin.OUT, Pin.PULL_UP), freq=80000)

while True:
    led.set_brightness(led_brightness)
    # for x in range(8):
    print("LED")
    for x in range(16):
        led.select_led(led_channels[x])
        led.on()
        # time.sleep_ms(80)
        time.sleep_ms(250)
