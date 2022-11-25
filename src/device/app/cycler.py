import micropython
from machine import Timer
micropython.alloc_emergency_exception_buf(100)
import time
import random
import json

# The main logic of thermal and optical units

class DeviceState:
    def __init__(self, label, has_experiment=False, start_available=False, 
    resume_available=False, pause_available=False, cancel_available=False, finish_available=False):
        self.label = label
        self.has_experiment = has_experiment
        self.start_available = start_available
        self.resume_available = resume_available
        self.pause_available = pause_available
        self.cancel_available = cancel_available
        self.finish_available = finish_available
    def data(self):
        print("DeviceState.data()")
        return {
            "b":self.label, # label
            "x":self.has_experiment, # has_experiment
            "i":self.start_available, # start_available
            "r":self.resume_available, # resume_available
            "u":self.pause_available, # pause_available
            "c":self.cancel_available, # cancel_available
            "f":self.finish_available # finish_available
        }
TEMP_CONTROL_INTERVAL_MSEC = 500
DEFAULT_TEMP = 25
MAX_TEMP_RATE = 2.0
class TempControlSimulator:
    def __init__(self, scheduler):
        self.schedule = scheduler.add_schedule()
        self.temp = DEFAULT_TEMP
        self.target_temp = DEFAULT_TEMP
    def reset (self):
        pass
    def off (self):
        print("TempControlSimulator.OFF")
    def control (self):
        self.termistor_index = 0
        self.schedule.init_timer(200, Timer.PERIODIC, self.measure_next)
    def get_temp(self):
        return self.temp
    def get_well_temp(self):
        return self.temp
    def get_sample_temp(self):
        return self.temp - 0.8
    def get_air_temp(self):
        return 25
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

CHANNEL_COUNT = 2
WELL_COUNT = 8

class OpticsSimulator:
    def __init__(self, scheduler):
        self.schedule = scheduler.add_schedule()
        self.is_measuring = False
        self.channel_index = 0
        self.well_index = 0
    def measure_all(self, callback):
        print("measure_all")
        if self.is_measuring:
            return False # Rejected (busy)
        self.callback = callback
        self.channel_index = 0
        self.well_index = 0
        self.is_measuring = True
        self.measurement = []
        for i in range(CHANNEL_COUNT):
            self.measurement.append([0] * WELL_COUNT)
        self.schedule.init_timer(200, Timer.PERIODIC, self.measure_next)
        return True # Accepted
    def measure_next (self):
        # Simulation
        self.measurement[self.channel_index][self.well_index] = random.random()
        if self.well_index == WELL_COUNT - 1:
            # Next channel & first well
            if self.channel_index == CHANNEL_COUNT - 1:
                # All done
                self.is_measuring = False
                self.schedule.cancel_timer()
                self.callback(self.measurement)
            self.well_index = 0
            self.channel_index += 1
        else:
            # Next well
            self.well_index += 1

STATE_IDLE = DeviceState("idle", start_available=True)
STATE_RUNNING = DeviceState("running", has_experiment=True, pause_available=True, cancel_available=True)
STATE_PAUSED = DeviceState("paused", has_experiment=True, resume_available=True, cancel_available=True)
STATE_COMPLETE = DeviceState("complete", has_experiment=True, cancel_available=True, finish_available=True)

class Cycler:
    def __init__(self, temp_control, optics, communicator, scheduler):
        self.schedule = scheduler.add_schedule()
        self.temp_control = temp_control
        self.optics = optics
        self.communicator = communicator
        self.current_step = None
        self.temperature = 22.222
        self.started = False
        self.paused = False
        self.timestamp_ms = 0
        self.step_timestamp_ms = 0
        self.protocol = None
        self.state = STATE_IDLE
        self.experiment_id = ""
        self.schedule.init_timer(TEMP_CONTROL_INTERVAL_MSEC, Timer.PERIODIC, self.periodic)
    
    def start(self, protocol, experiment_id=""):
        print("start.")
        self.experiment_id = experiment_id
        if self.protocol != None:
            self.communicator.on_error("Already started")
            return False
        self.temp_control.reset()
        self._timestamp_init()
        self.protocol = protocol
        self.started = True
        self.step_index = None
        # self.schedule.init_timer(TEMP_CONTROL_INTERVAL_MSEC, Timer.PERIODIC, self.progress)
        self.state = STATE_RUNNING
        self.next_stage()
        self.communicator.on_event("start", data={"p":self.protocol.protocol})
        self._publish_state()
        return True

    # stopwatch
    def _timestamp_init (self):
        self.timestamp_ms = 0
        self.step_timestamp_ms = 0
        self.last_tick = time.ticks_ms()

    def _timestamp_update (self):
        if self.paused == False:
            self.timestamp_ms += time.ticks_ms() - self.last_tick
            self.step_timestamp_ms += time.ticks_ms() - self.last_tick
        self.last_tick = time.ticks_ms()

    def _publish_state(self):
        self.communicator.on_device_state_change(self.state.data())

    # Commands
    def resume (self):
        if self.protocol == None:
            self.communicator.on_error("No experiment")
            return False
        if self.paused == False:
            self.communicator.on_error("Still running")
            return False
        # Resume heating or cooling
        self.temp_control.set_target_temp(self.current_step.target_temp)
        self.paused = False
        self.communicator.on_event("resume", data={"protocol":self.protocol.protocol})
        self.state = STATE_RUNNING
        self._publish_state()
        return True

    def cancel (self):
        if self.protocol == None:
            self.communicator.on_error("No experiment")
            return False
        # self.schedule.cancel_timer()
        self.temp_control.set_target_temp(None)
        self.protocol = None
        self.communicator.on_event("cancel", data={"protocol":self.protocol.protocol})
        self.state = STATE_IDLE
        self._publish_state()
        return True
    
    def finish (self):
        if self.protocol == None:
            self.communicator.on_error("No experiment")
            return False
        if self.current_step.is_finished() == False:
            self.communicator.on_error("Still running")
            return False
        # self.schedule.cancel_timer()
        self.temp_control.set_target_temp(None)
        self.communicator.on_event("finish", data={"protocol":self.protocol.protocol})
        self.protocol = None
        self.state = STATE_IDLE
        self._publish_state()
        return True

    def pause (self):
        # Keep current temperature
        if self.protocol == None:
            self.communicator.on_error("No experiment")
            return False
        if self.paused == True:
            self.communicator.on_error("Already paused")
            return False
        self.temp_control.set_target_temp(self.temperature)
        self.paused = True
        self.communicator.on_event("pause", data={"protocol":self.protocol.protocol})
        self.state = STATE_PAUSED
        self._publish_state()
        return True

    def next_stage (self):
        print("###### next_stage ######")
        self.step_timestamp_ms = 0
        if self.step_index == None:
            self.step_index = 0
        else:
            self.step_index += 1
        self.current_step = self.protocol.steps[self.step_index]
        self.current_step.start(self.temperature, self.timestamp_ms)
        self.temp_control.set_target_temp(self.current_step.target_temp)
        self.last_measurement = None

        if self.current_step.is_finished():
            self.communicator.on_event("complete", data={"protocol":self.protocol.protocol})
            self.state = STATE_COMPLETE
            self._publish_state()
    def periodic(self):
        self.temp_control.control()
        if self.protocol:
            self.progress()
    def progress(self):
        self._timestamp_update()
        temp = self.temp_control.get_temp()
        self.temperature = temp
        if self.started == False:
            return
        if (self.current_step.is_done(temp, self.timestamp_ms)):
            self.next_stage()
        if self.current_step.min_measurement_interval != None:
            if self.last_measurement == None or (self.timestamp_ms - self.last_measurement) > self.current_step.min_measurement_interval * 1000:
                if self.optics.measure_all(self.optics_on_measure):
                    # Optics.measure_all returns False if it has ongoing measurement
                    self.last_measurement = self.timestamp_ms
        state = {"state":self.current_step.label}
        self.communicator.on_progress({
            "e":self.timestamp_ms, # elapsed
            "s": self.current_step.index, #step
            "d":self.step_timestamp_ms, # step_elapsed
            "l": self.current_step.label, #step_label
            "p": self.temperature, # p
            "a": self.temp_control.get_air_temp(), # air
            "m":self.temp_control.get_sample_temp() # sample
        })

    def optics_on_measure(self, data):
        self.communicator.on_measure({"e":self.timestamp_ms,"s": self.current_step.index,"d":self.step_timestamp_ms,"v":data})

    def request_state(self):
        self._publish_state()
    def request_protocol (self):
        if self.protocol is not None:
            self.communicator.response_protocol(self.protocol.raw)


TEMP_TOLERANCE = 0.5        

class ExperimentProtocol:
    def __init__(self, profile=None):
        self.steps = []
        self.raw = profile
        self.protocol = profile
        if str:
            try:
                print(str)
                steps = profile["steps"]
                index = 0
                for step in steps:
                    temp = step["t"]
                    duration = step["d"]
                    self.add_step(StepRamp(target_temp=temp, index=index))
                    if step.get("c", 1):
                        self.add_step(StepHold(target_temp=temp, hold_sec=duration, index=index, min_measurement_interval=step.get("i", None)))
                    else:
                        self.add_step(StepHold(target_temp=temp, hold_sec=duration, index=index))
                    index += 1
                self.add_step(StepFinalHold(target_temp=profile["h"], index=index))
            except Exception as e:
                print(e)
                raise Exception("Malformed protocol string")
            print(["Steps count", len(self.steps)])
    def add_step(self, stage):
        self.steps.append(stage)

class StepRamp:
    def __init__(self, target_temp, index=0):
        self.target_temp = target_temp
        self.initial_temp = 0
        self.label = "ramp"
        self.min_measurement_interval = None
        self.index = index
    def is_done(self, measured_temp, timestamp_ms):
        if self.initial_temp < self.target_temp:
            # Heating ramp
            return measured_temp >= self.target_temp - TEMP_TOLERANCE
        else:
            # Cooling ramp
            return measured_temp <= self.target_temp + TEMP_TOLERANCE
    def start(self, measured_temp, timestamp_ms):
        self.initial_temp = measured_temp
        print(["StepRamp Start", self.initial_temp, self.target_temp])
    def is_finished(self):
        return False
    def obj (self):
        return {"label":"ramp","index":self.index}

class StepHold:
    def __init__(self, target_temp, hold_sec, min_measurement_interval=None, index=0):
        self.target_temp = target_temp
        self.hold_sec = hold_sec
        self.label = "hold"
        self.start_time = None
        self.min_measurement_interval = min_measurement_interval
        self.index = index
    def is_done(self, measured_temp, timestamp_ms):
        elapsed_ms = timestamp_ms - self.start_time
        return elapsed_ms > self.hold_sec * 1000
    def start(self, measured_temp, timestamp_ms):
        self.start_time = timestamp_ms
    def is_finished(self):
        return False
    def obj (self):
        return {"label":"hold","index":self.index}

class StepFinalHold:
    def __init__(self, target_temp=None, index=0):
        self.target_temp = target_temp
        self.label = "final_hold"
        self.min_measurement_interval = None
        self.index = index
    def is_done(self, measured_temp, timestamp_ms):
        return False # Never ends
    def start(self, measured_temp, timestamp_ms):
        pass
    def is_finished(self):
        return True
    def obj (self):
        return {"label":"final_hold","index":self.index}
