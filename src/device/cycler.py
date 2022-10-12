import micropython
from machine import Timer
micropython.alloc_emergency_exception_buf(100)
import time
import random
import json

# The main logic of thermal and optical units

class DeviceState:
    def __init__(self, label, hasExperiment=False, startAvailable=False, 
    resumeAvailable=False, pauseAvailable=False, cancelAvailable=False, finishAvailable=False):
        self.label = label
        self.hasExperiment = hasExperiment
        self.startAvailable = startAvailable
        self.resumeAvailable = resumeAvailable
        self.pauseAvailable = pauseAvailable
        self.cancelAvailable = cancelAvailable
        self.finishAvailable = finishAvailable
    def data(self):
        print("DeviceState.data()")
        return {
            "label":self.label,
            "hasExperiment":self.hasExperiment,
            "startAvailable":self.startAvailable,
            "resumeAvailable":self.resumeAvailable,
            "pauseAvailable":self.pauseAvailable,
            "cancelAvailable":self.cancelAvailable,
            "finishAvailable":self.finishAvailable
        }
TEMP_CONTROL_INTERVAL_MSEC = 1000
DEFAULT_TEMP = 25
MAX_TEMP_RATE = 2.0
class TempControlSimulator:
    def __init__(self, scheduler):
        self.schedule = scheduler.add_schedule()
        self.temp = DEFAULT_TEMP
        self.target_temp = DEFAULT_TEMP
    def control (self):
        self.termistor_index = 0
        self.schedule.init_timer(200, Timer.PERIODIC, self.measure_next)
    def get_temp(self):
        return self.temp
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

STATE_IDLE = DeviceState("idle", startAvailable=True)
STATE_RUNNING = DeviceState("running", hasExperiment=True, pauseAvailable=True, cancelAvailable=True)
STATE_PAUSED = DeviceState("paused", hasExperiment=True, resumeAvailable=True, cancelAvailable=True)
STATE_AUTO_PAUSED = DeviceState("auto_paused", hasExperiment=True, resumeAvailable=True, cancelAvailable=True)
STATE_COMPLETE = DeviceState("complete", hasExperiment=True, cancelAvailable=True, finishAvailable=True)

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
    
    def start(self, protocol, experiment_id=""):
        print("start.")
        self.experiment_id = experiment_id
        if self.protocol != None:
            self.communicator.on_error("Already started")
            return
        self._timestamp_init()
        self.protocol = protocol
        self.started = True
        self.step_index = None
        self.schedule.init_timer(TEMP_CONTROL_INTERVAL_MSEC, Timer.PERIODIC, self.progress)
        self.state = STATE_RUNNING
        self.next_stage()
        self._publish_state()

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
            return
        if self.paused == False:
            self.communicator.on_error("Still running")
            return
        # Resume heating or cooling
        self.temp_control.set_target_temp(self.current_step.target_temp)
        self.paused = False
        self.communicator.on_event("resume")
        self.state = STATE_RUNNING
        self._publish_state()

    def cancel (self):
        if self.protocol == None:
            self.communicator.on_error("No experiment")
            return
        self.schedule.cancel_timer()
        self.temp_control.set_target_temp(None)
        self.protocol = None
        self.communicator.on_event("cancel")
        self.state = STATE_IDLE
        self._publish_state()
    
    def finish (self):
        if self.protocol == None:
            self.communicator.on_error("No experiment")
            return
        if self.current_step.is_finished() == False:
            self.communicator.on_error("Still running")
            return
        self.schedule.cancel_timer()
        self.temp_control.set_target_temp(None)
        self.protocol = None
        self.communicator.on_event("finish")
        self.state = STATE_IDLE
        self._publish_state()

    def pause (self):
        # Keep current temperature
        if self.protocol == None:
            self.communicator.on_error("No experiment")
            return
        if self.paused == True:
            self.communicator.on_error("Already paused")
            return
        self.temp_control.set_target_temp(self.temperature)
        self.paused = True
        self.communicator.on_event("pause")
        self.state = STATE_PAUSED
        self._publish_state()

    def next_stage (self):
        print("###### next_stage ######")
        self.step_timestamp_ms = 0
        data = {}

        if self.current_step != None:
            data["from"] = self.current_step.obj()
        if self.step_index == None:
            self.step_index = 0
        else:
            self.step_index += 1
        self.current_step = self.protocol.steps[self.step_index]
        self.current_step.start(self.temperature, self.timestamp_ms)
        self.temp_control.set_target_temp(self.current_step.target_temp)
        self.last_measurement = None
        data["to"] = self.current_step.obj()
        self.communicator.on_transition(data)
        if self.current_step.is_finished():
            self.state = STATE_COMPLETE
            self._publish_state()

    def progress(self):
        self._timestamp_update()
        self.temp_control.control()
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
            "elapsed":self.timestamp_ms,
            "step_elapsed":self.step_timestamp_ms,
            "step":self.current_step.obj(),
            "target":self.current_step.target_temp,
            "plate":self.temperature, "state":state
        })
        self.communicator.loop()

    def optics_on_measure(self, data):
        self.communicator.on_measure({"elapsed":self.timestamp_ms,"step_elapsed":self.step_timestamp_ms,"v":data})

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
        if str:
            try:
                print(str)
                steps = profile["steps"]
                index = 0
                for step in steps:
                    temp = step["temp"]
                    duration = step["duration"]
                    self.add_step(StepRamp(target_temp=temp, index=index))
                    if step.get("data_collection", 1):
                        self.add_step(StepHold(target_temp=temp, hold_sec=duration, index=index, min_measurement_interval=step.get("data_collection_interval", None)))
                    else:
                        self.add_step(StepHold(target_temp=temp, hold_sec=duration, index=index))
                    index += 1
                self.add_step(StepFinalHold(target_temp=profile["final_hold_temp"]))
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
            return measured_temp <= self.target_temp + TEMP_CONTROL_INTERVAL_MSEC
    def start(self, measured_temp, timestamp_ms):
        self.initial_temp = measured_temp
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
    def __init__(self, target_temp=None):
        self.target_temp = target_temp
        self.label = "final_hold"
        self.min_measurement_interval = None
    def is_done(self, measured_temp, timestamp_ms):
        return False # Never ends
    def start(self, measured_temp, timestamp_ms):
        pass
    def is_finished(self):
        return True
    def obj (self):
        return {"label":"final_hold","index":0}
