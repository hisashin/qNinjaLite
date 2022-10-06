import micropython
from machine import Timer
from scheduler import Scheduler, Schedule
micropython.alloc_emergency_exception_buf(100)

import time

scheduler = Scheduler()

class TempControl:
    def __init__(self):
        self.schedule = scheduler.add_schedule()
    def control (self):
        print("TempControl.control")
        self.well_index = 0
        self.schedule.init_timer(200, Timer.PERIODIC, self.measure_next)
    def measure_next (self):
        print(["measure", self.well_index])
        if self.well_index == 3:
            self.schedule.cancel_timer()
        self.well_index += 1
        
class Optics:
    def __init__(self):
        self.schedule = scheduler.add_schedule()

class PCR:
    def __init__(self, temp_control, optics):
        self.schedule = scheduler.add_schedule()
        self.temp_control = temp_control
        self.optics = optics
    def start(self):
        self.schedule.init_timer(4000, Timer.PERIODIC, self.temp_control.control)

temp_control = TempControl()
optics = Optics()
pcr = PCR(temp_control, optics)
pcr.start()

while True:
    scheduler.loop()
    time.sleep(0.01)