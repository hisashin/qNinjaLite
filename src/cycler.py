import micropython
from machine import Timer
from scheduler import Scheduler, Schedule
micropython.alloc_emergency_exception_buf(100)

import time

TEMP_CONTROL_INTERVAL_MSEC = 1000

scheduler = Scheduler()
MAX_TEMP_RATE = 2.0
class TempControl:
    def __init__(self):
        self.schedule = scheduler.add_schedule()
        self.temp = 25
        self.target_temp = 25
    def control (self):
        print("TempControl.control")
        self.well_index = 0
        self.schedule.init_timer(200, Timer.PERIODIC, self.measure_next)
    def measure_next (self):
        print(["measure", self.well_index])
        if self.well_index == 0:
            if self.target_temp > self.temp:
                self.temp = min(self.temp + MAX_TEMP_RATE, self.target_temp)
            elif self.target_temp < self.temp:
                self.temp = max(self.temp - MAX_TEMP_RATE, self.target_temp)
        if self.well_index == 3:
            self.schedule.cancel_timer()
        self.well_index += 1
    def set_target_temp(self, temp):
        self.target_temp = temp
        print("setTargetTemp", self.target_temp)
        
class Optics:
    def __init__(self):
        self.schedule = scheduler.add_schedule()

PHASE_RAMPING = 1
PHASE_HOLDING = 2
FINISHED = 3
class PCR:
    def __init__(self, temp_control, optics):
        self.schedule = scheduler.add_schedule()
        self.temp_control = temp_control
        self.optics = optics
        self.holding_time = 0
    
    def start(self, protocol):
        self.protocol = protocol
        self.stage_index = None
        self.schedule.init_timer(TEMP_CONTROL_INTERVAL_MSEC, Timer.PERIODIC, self.progress)
        self.next_stage()
        self.phase = PHASE_RAMPING
        self.finished = False
    def next_stage (self):
        print("next_stage")
        if self.stage_index == None:
            self.stage_index = 0
            self.phase = PHASE_RAMPING
            self.temp_control.set_target_temp(self.protocol.stages[self.stage_index].target_temp)
            self.holding_time = 0
        elif self.stage_index < len(self.protocol.stages)-1:
            self.stage_index += 1
            self.phase = PHASE_RAMPING
            self.temp_control.set_target_temp(self.protocol.stages[self.stage_index].target_temp)
            self.holding_time = 0
        else:
            self.finished = True
            print("Finish")
            self.phase = FINISHED
            self.holding_time = 0

    def progress(self):
        self.temp_control.control()
        if self.phase == FINISHED:
            pass
        else:
            stage = self.protocol.stages[self.stage_index]
            target_temp = stage.target_temp
        print(["Phase", self.phase])
        if self.phase == PHASE_RAMPING:
            if abs(self.temp_control.temp - target_temp) < 1:
                print("Hold")
                self.phase = PHASE_HOLDING
                
        elif self.phase == PHASE_HOLDING:
            self.holding_time += 2 # TODO
            print([self.holding_time, stage.hold_sec])
            if self.holding_time >= stage.hold_sec:
                print("Next")
                self.next_stage()
        # Pick stage if none (set target temp and rate)
        # If ramping, compare target temp
        # If holding, check elapsed time
        print(self.temp_control.temp)
        

temp_control = TempControl()
optics = Optics()
pcr = PCR(temp_control, optics)


class ThermalProtocol:
    def __init__(self):
        self.stages = []
    def add_stage(self, stage):
        self.stages.append(stage)
class ThermalProtocolStage:
    def __init__(self, target_temp, hold_sec, ramp_rate=0):
        self.target_temp = target_temp
        self.hold_sec = hold_sec
        self.ramp_rate = ramp_rate

protocol = ThermalProtocol()
stage_a = ThermalProtocolStage(target_temp=40, hold_sec=20)
stage_b = ThermalProtocolStage(target_temp=80, hold_sec=20, ramp_rate=5)
protocol.add_stage(stage_a)
protocol.add_stage(stage_b)

pcr.start(protocol)

while True:
    scheduler.loop()
    time.sleep(0.01)


