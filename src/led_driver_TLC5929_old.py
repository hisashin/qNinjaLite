from machine import Pin, SPI, SoftSPI
import time
print("Init SPI")
# SCLK=12, MOSI=13, MISO=15, LAT=14
sclk=Pin(12,Pin.OUT)
mosi=Pin(13,Pin.OUT)

blank=Pin(15,Pin.OUT)
sclk.off()
mosi.off()
lat=Pin(14,Pin.OUT)
lat.off()
blank.off()
print("Init SPI...")
spi = SoftSPI(baudrate=10000, polarity=0, phase=0, firstbit=SPI.MSB, sck=sclk, mosi=mosi, miso=Pin(16))
print("Init SPI done")
blank.off()

def latch ():
    lat.on()
    time.sleep(0.0001)
    lat.off()

def read ():
    time.sleep(1)
    lat.on()
    time.sleep(0.01)
    lat.off()
    time.sleep(0.01)
    rxdata = bytearray(2)
    spi.readinto(rxdata, 0x00) 

def rev (v):
    rev = 0
    for i in range(8):
        rev <<= 1
        rev += v & 1
        v >>= 1
    return rev

def send (msg):
    time.sleep(0.25)
    latch()
    spi.write(msg)
    latch()
    time.sleep(0.25)


def set_brightness (brightness) :
    brightness_rev = 0xFF & brightness
    send(bytearray([0x01, 0x00, brightness_rev]))

def select_led (selected_led):
    vl = 0x00
    vh = 0x00
    if selected_led < 8:
        vl = 0xFF & ( 1 << selected_led )
    else:
        vh = 0xFF & ( 1 << (selected_led - 8) )
    send(bytearray([0x00, vh, vl]))

def select_all ():
    send(bytearray([0x00, 0xFF, 0xFF]))

def on () :
    blank.off()

def off () :
    blank.on()
