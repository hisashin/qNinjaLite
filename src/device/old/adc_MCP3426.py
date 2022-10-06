# MCP3426

import time
from machine import Pin, SoftI2C

device_address = 0x68
scl = Pin(18, Pin.OUT, Pin.PULL_UP)
sda = Pin(5, Pin.OUT, Pin.PULL_UP)
drdy = Pin(17, Pin.IN, Pin.PULL_UP)
i2c = SoftI2C(scl, sda, freq=80000)
print("Scanning...")
devices = i2c.scan()
print(devices)
print("Scan done.")

def read_conf_reg ():
    print("read_conf_reg")
    reg = i2c.readfrom(device_address, 3)
    print([bin(reg[2])])

def read_conversion_data ():
    reg = i2c.readfrom(device_address, 3)
    print(reg[0]<<8|reg[1])

def start ():
    print("Start!!!")
read_conf_reg()