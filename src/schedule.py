import micropython
micropython.alloc_emergency_exception_buf(100)
# https://docs.micropython.org/en/latest/wipy/tutorial/timer.html
import time
while True:
    print("Schedule.")
    time.sleep(1)