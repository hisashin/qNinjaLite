import time
from machine import Pin, PWM


duty = 255
mux_s0 = Pin(14, Pin.OUT)
mux_s1 = Pin(12, Pin.OUT)
mux_s2 = Pin(22, Pin.OUT)
mux_s3 = Pin(23, Pin.OUT)
pwm = PWM(Pin(25,Pin.OUT), freq=1000)

def select_mux (ch): 
    val0 = 0x01 & (ch >> 0)
    val1 = 0x01 & (ch >> 1)
    val2 = 0x01 & (ch >> 2)
    val3 = 0x01 & (ch >> 3)
    mux_s0.value(val0)
    mux_s1.value(val1)
    mux_s2.value(val2)
    mux_s3.value(val3)
    print([val0, val1, val2, val3])

num = 0
while True:
    time.sleep(1)
    pwm.duty(512)
    duty -= 256
    if (duty < 0):
        duty = 1023
    select_mux(num)
    num = (num + 1) % 4