import micropython
from machine import Timer
from scheduler import Scheduler, Schedule
micropython.alloc_emergency_exception_buf(100)
from ws import WebSocketServer
import time
import random

TEMP_CONTROL_INTERVAL_MSEC = 1000
DEFAULT_TEMP = 25
scheduler = Scheduler()
MAX_TEMP_RATE = 2.0
class TempControl:
    def __init__(self):
        self.schedule = scheduler.add_schedule()
        self.temp = DEFAULT_TEMP
        self.target_temp = DEFAULT_TEMP
    def control (self):
        self.termistor_index = 0
        self.schedule.init_timer(200, Timer.PERIODIC, self.measure_next)
    def measure_next (self):
        # print(["measure", self.termistor_index])
        # Simulation
        if self.termistor_index == 0:
            if self.target_temp == None:
                # Just dissipate
                rate = min(0.5, (self.temp - DEFAULT_TEMP)/20.0)
                self.temp = max(self.temp - rate, DEFAULT_TEMP)
            elif self.target_temp > self.temp:
                self.temp = min(self.temp + MAX_TEMP_RATE, self.target_temp)
            elif self.target_temp < self.temp:
                self.temp = max(self.temp - MAX_TEMP_RATE, self.target_temp)
        if self.termistor_index == 3:
            self.schedule.cancel_timer()
        self.termistor_index += 1
    def set_target_temp(self, temp):
        self.target_temp = temp
        print("setTargetTemp", self.target_temp)
        # TODO use PID object
WELL_COUNT = 4
class Optics:
    def __init__(self):
        self.schedule = scheduler.add_schedule()
        self.is_measuring = False
    def measure_all(self, callback):
        print("measure_all")
        if self.is_measuring:
            return False # Rejected (busy)
        self.callback = callback
        self.well_index = 0
        self.is_measuring = True
        self.measurement = [0] * WELL_COUNT
        self.schedule.init_timer(200, Timer.PERIODIC, self.measure_next)
        return True # Accepted
    def measure_next (self):
        # Simulation
        self.measurement[self.well_index] = random.random()
        if self.well_index == WELL_COUNT - 1:
           self.is_measuring = False
           self.schedule.cancel_timer()
           self.callback(self.measurement)
        self.well_index += 1

class Communicator:
    def __init__(self, server):
        self.server = server
    def start(self):
        self.server.start()
    def on_progress(self, temp):
        print("Temp={temp}".format(temp=temp))
        self.server.send("Temp={temp}".format(temp=temp))
    def on_transition(self):
        pass
    def on_measure(self):
        pass
    def loop(self):
        msg = self.server.read()
        if msg:
            # Received message
            print(msg)
# Message format? 

class Cycler:
    def __init__(self, temp_control, optics, communicator):
        self.schedule = scheduler.add_schedule()
        self.temp_control = temp_control
        self.optics = optics
        self.communicator = communicator
        self.current_step = None
        self.communicator.start()
        self.temperature = 22.222
        self.started = False
    
    def start(self, protocol):
        self.protocol = protocol
        self.started = True
        self.step_index = None
        self.schedule.init_timer(TEMP_CONTROL_INTERVAL_MSEC, Timer.PERIODIC, self.progress)
        self.next_stage()

    def next_stage (self):
        print("next_stage")
        if self.step_index == None:
            self.step_index = 0
        else:
            self.step_index += 1
        self.current_step = self.protocol.steps[self.step_index]
        self.current_step.start(self.temperature)
        self.temp_control.set_target_temp(self.current_step.target_temp)
        self.last_measurement = None

    def progress(self):
        self.temp_control.control()
        temp = self.temp_control.temp
        self.temperature = temp
        if self.started == False:
            return
        if (self.current_step.is_done(temp)):
            self.next_stage()
        tick = time.ticks_ms()
        if self.current_step.min_measurement_interval != None:
            if self.last_measurement == None or (tick - self.last_measurement) > self.current_step.min_measurement_interval * 1000:
                if optics.measure_all(self.optics_on_measure):
                    # Optics.measure_all returns False if it has ongoing measurement
                    self.last_measurement = tick
        self.communicator.on_progress(self.temperature)
        self.communicator.loop()

    def optics_on_measure(self, data):
        print(data)
        pass
        

temp_control = TempControl()
optics = Optics()
server = WebSocketServer()
communicator = Communicator(server)
pcr = Cycler(temp_control, optics, communicator)

class ThermalProtocol:
    def __init__(self):
        self.steps = []
    def add_step(self, stage):
        self.steps.append(stage)

TEMP_TOLERANCE = 0.
class StepRamp:
    def __init__(self, target_temp):
        self.target_temp = target_temp
        self.initial_temp = 0
        self.label = "ramp"
        self.min_measurement_interval = None
    def is_done(self, measured_temp):
        if self.initial_temp < self.target_temp:
            # Heating ramp
            return measured_temp >= self.target_temp - TEMP_TOLERANCE
        else:
            # Cooling ramp
            return measured_temp <= self.target_temp + TEMP_CONTROL_INTERVAL_MSEC
    def start(self, measured_temp):
        self.initial_temp = measured_temp

class StepHold:
    def __init__(self, target_temp, hold_sec, min_measurement_interval=None):
        self.target_temp = target_temp
        self.hold_sec = hold_sec
        self.label = "hold"
        self.start_time = None
        self.min_measurement_interval = min_measurement_interval
    def is_done(self, measured_temp):
        elapsed_msec = time.ticks_ms() - self.start_time
        return elapsed_msec > self.hold_sec * 1000
    def start(self, measured_temp):
        self.start_time = time.ticks_ms()

class StepFinish:
    def __init__(self, target_temp=None):
        self.target_temp = target_temp
        self.label = "finish"
        self.min_measurement_interval = None
    def is_done(self, measured_temp):
        return False # Never ends
    def start(self, measured_temp):
        pass

protocol = ThermalProtocol()
protocol.add_step(StepRamp(target_temp=50))
protocol.add_step(StepHold(target_temp=50, hold_sec=20, min_measurement_interval=5))
protocol.add_step(StepRamp(target_temp=60))
protocol.add_step(StepHold(target_temp=60, hold_sec=20, min_measurement_interval=4))
protocol.add_step(StepFinish())

print("Init WebSocketServer")
debug_count = 0

while True:
    scheduler.loop()
    time.sleep(0.01)
    debug_count += 1
    if debug_count == 400:
        pcr.start(protocol)


