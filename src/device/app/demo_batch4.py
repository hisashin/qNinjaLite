# Hardware control for batch4 board
# - For optics demo

from hardware_batch4 import TempControl, Optics
from machine import Timer
from scheduler import Scheduler
import time

TARGET_TEMP = 34

count = 0
scheduler = Scheduler()
# temp_control = TempControl(scheduler, measure_interval_ms=100, pid_interval_ms=1000)
# temp_control.set_target_temp(TARGET_TEMP)
optics = Optics(scheduler, measure_interval_ms=200)

def optics_on_measure(values):
    print(str(optics.get_brightness()) + "\t" + "\t".join(map(str, values[0])))
def progress ():
    global count
    # temp_control.control()
    count += 1
    if count == 2:
        optics.measure_all(optics_on_measure)
        count = 0

CONTROL_INTERVAL_MSEC = 1000
CONTROL_INTERVAL_SEC = CONTROL_INTERVAL_MSEC/1000.0

control_schedule = scheduler.add_schedule()
control_schedule.init_timer(CONTROL_INTERVAL_MSEC, Timer.PERIODIC, progress)

while True:
    scheduler.loop()
    time.sleep(0.01)
