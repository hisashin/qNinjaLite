from cycler import *
from mqtt import MQTTClient
from scheduler import Scheduler
import machine

import socket
import json
import cycler_mqtt_config

scheduler = Scheduler()
demo_thing_id = cycler_mqtt_config.demo_thing_id
server = cycler_mqtt_config.server
port = 8883 #secure

import ssl

keyfile = "certs/thing_private.pem.key"
certfile = "certs/thing_certificate.pem.crt"
ca_certs = "certs/CA1.cer"


class NetworkMQTTClient:
    def __init__(self):
        cert = ""
        key = ""
        with open(keyfile, 'rb') as f:
            key = f.read()
        with open(certfile, 'rb') as f:
            cert = f.read()
        print(key)
        print(cert)
        addr_info = socket.getaddrinfo(server, port)
        print(addr_info)
        addr = addr_info[0][-1]

        # Init SSL
        skt = socket.socket()
        # s = skt
        print("Connecting...")
        skt.connect(addr)
        print("Connected")
        global s
        s = ssl.wrap_socket(skt, cert=cert, key=key)
        print("Wrapped")
        self.socket = s
        s.setblocking(False)
    def time_ms(self):
        return time.ticks_ms()
    def read(self, length):
        return self.socket.read(length)
    def write (self, packet):
        self.socket.write(packet)

mqttclient = MQTTClient(NetworkMQTTClient())
mqttclient.connect()

# ESP32 works as WebSoekct Server
class MQTTCommunicator:
    def __init__(self):
        pass
    def start(self):   
        time.sleep(0.5)
        print("Sub cmd")
        mqttclient.subscribe("cmd/qninjalite/#")
        time.sleep(0.5)
        ping_topic =  "cmd/qninjalite/" + self._thing_id() + "/ping-device"
        mqttclient.publish(ping_topic,"{}", qos=0)

    def _thing_id (self):
        return demo_thing_id
    def _experiment_id (self):
        return cycler.experiment_id

    def _experiment_data_topic (self, command):
        return "dt/qninjalite/" + self._thing_id() + "/experiment/" + self._experiment_id() + "/" + command
    def _device_data_topic (self, command):
        return "dt/qninjalite/" + self._thing_id() + "/" + command

    def _parse_command_topic (self, topic):
        levels = topic.split("/")
        return levels[-1]
    def _respond_to_command(self, topic, req_data, res_data=None, accepted=True, req_id_needed=False):
        res_topic = topic + "-res"
        req_id = req_data.get("req_id", None)
        print(["RESPONSE_TO_TOPIC", res_topic])
        if req_id == None and req_id_needed == False:
            print("no req id.")
            return # Reponse not needed
        req_data = {"accepted":accepted, "req_id":req_id}
        if res_data:
            req_data["data"] = res_data
        mqttclient.publish(res_topic, json.dumps(req_data), qos=0)


    def on_progress(self, data):
        # mqttclient.publish("update", json.dumps(data), qos=0)
        mqttclient.publish(self._experiment_data_topic("progress"), json.dumps(data), qos=0)
    def on_measure(self, data):
        mqttclient.publish(self._experiment_data_topic("fluo"), json.dumps(data), qos=0)
    def on_event(self, label):
        data = {"label":label}
        mqttclient.publish(self._experiment_data_topic("event"), json.dumps(data), qos=0)
    def on_device_state_change(self, data):
        mqttclient.publish(self._device_data_topic("state"),  json.dumps(data), qos=0)
    def response_protocol(self, data):
        mqttclient.publish(self._experiment_data_topic("protocol"),  json.dumps(data), qos=0)
    def on_message(self, message):
        print(message)
        fullTopic = message["topic"]
        topic = self._parse_command_topic(fullTopic)
        print(["on_message","topic", topic])

        # Experiment control
        if topic == "start":
            print("Start!!!")
            # { experiment_id:this._issueExperimentId(), protocol: experiment }
            print(message["message"])
            obj = json.loads(message["message"])
            accepted = cycler.start(ExperimentProtocol(profile=obj["protocol"]), experiment_id=obj["experiment_id"])
            self._respond_to_command(fullTopic, obj, accepted=accepted)
        elif topic == "cancel":
            accepted = cycler.cancel()
            self._respond_to_command(fullTopic, obj, accepted=accepted)
        elif topic == "pause":
            accepted = cycler.pause()
            self._respond_to_command(fullTopic, obj, accepted=accepted)
        elif topic == "resume":
            accepted = cycler.resume()
            self._respond_to_command(fullTopic, obj, accepted=accepted)
        elif topic == "finish":
            accepted = cycler.finish()
            self._respond_to_command(fullTopic, obj, accepted=accepted)

        # TODO Calibration
        elif topic == "calib":
            pass
        
        elif topic == "pid":
            obj = json.loads(message["message"])
            temp_control.set_pid_constants(obj["ranges"])
            accepted = True
            self._respond_to_command(fullTopic, obj, accepted=accepted)

        # State request
        elif topic == "req-state":
            print("req-state command received.")
            data = cycler.state.data()
            obj = json.loads(message["message"])
            print(["topic", topic])
            print(["obj", obj])
            print(["data", data])
            self._respond_to_command(fullTopic, obj, res_data=data)
        elif topic == "req-experiment":
            data = { "protocol":cycler.protocol.raw, "experiment_id":cycler.experiment_id }
            obj = json.loads(message["message"])
            self._respond_to_command(fullTopic, obj, res_data=data)

        # Ping
        elif topic == "ping-client":
            obj = json.loads(message["message"])
            # Should respond to ping-client even if req_id is empty.
            self._respond_to_command(fullTopic, obj, req_id_needed=True)
            
    def on_error(self, message):
        print("on_error")
        data = {"message":message}
        mqttclient.publish("error", json.dumps(data), qos=0)

    def loop(self):
        mqttclient.loop()

# Use dummy
if False:
    temp_control = TempControlSimulator(scheduler)
    optics = OpticsSimulator(scheduler)
# Use hardware

# if False:
from hardware_batch3 import TempControl, Optics, init_hardware
temp_control = TempControl(scheduler)
optics = Optics(scheduler)

communicator = MQTTCommunicator()
cycler = Cycler(temp_control, optics, communicator, scheduler)
mqttclient.set_on_message(communicator.on_message)
communicator.start()
wdt = machine.WDT(timeout=5000)  # enable it with a timeout of 5s
init_hardware()

print("Init MQTT Server")

count = 0
while True:
    if count % 1000 == 0:
        print("1000 loops A")
    scheduler.loop()
    time.sleep(0.005)
    if count % 1000 == 0:
        print("1000 loops B")
    communicator.loop()
    time.sleep(0.005)
    if count % 1000 == 0:
        print("1000 loops C")
    count += 1
    wdt.feed()
