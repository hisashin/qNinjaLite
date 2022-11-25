
from cycler import *
from scheduler import Scheduler
import time

import json
scheduler = Scheduler()

# For offline demo
class OfflineCommunicator:
    def __init__(self):
        pass
    def start(self):
        pass

    def on_progress(self, data):
        # mqttclient.publish("update", json.dumps(data), qos=0)
        print(json.dumps(data))
    def on_measure(self, data):
        print(json.dumps(data))
    def on_event(self, label, data={}):
        data = {"label":label}
        print(json.dumps(data))
    def on_device_state_change(self, data):
        print(json.dumps(data))
    def response_protocol(self, data):
        print(json.dumps(data))
    def on_message(self, message):
        pass
            
    def on_error(self, message):
        print("on_error")
        print(json.dumps(message))

    def loop(self):
        pass

if False:
    temp_control = TempControlSimulator(scheduler) # From cycler.py
    optics = OpticsSimulator(scheduler) # From cycler.py


# Hardware
# time.sleep(3.0) #?
from hardware_batch3 import TempControl, Optics, init_hardware
time.sleep(3.0) # Heating!
temp_control = TempControl(scheduler, measure_interval_ms=100, pid_interval_ms=1000)
optics = Optics(scheduler, measure_interval_ms=200)
init_hardware()
# Heating!!! time.sleep(3.0)

communicator = OfflineCommunicator()
cycler = Cycler(temp_control, optics, communicator, scheduler)
communicator.start()

print("Init Offine Experiment Demo.")
debug_count = 0
protocol_obj = {
    "name":"Default Protocol",
    "steps":[
        {"temp":50,"duration":30.0,"data_collection":1,"data_collection_interval":5},
        {"temp":66,"duration":30.0,"data_collection":1,"data_collection_interval":5}
        ],
    "h":20
}
protocol = ExperimentProtocol(profile=protocol_obj)

print("Starting Offine Experiment Demo.")
while True:
    scheduler.loop()
    time.sleep(0.01)
    debug_count += 1
    communicator.loop()
    if debug_count == 400:
        cycler.start(protocol)
