
from mqtt import MQTTClient
import socket
import time
server = 'localhost'

class NetworkLocal:
    def __init__(self):
        addr_info = socket.getaddrinfo(server, 1883)
        print(addr_info)
        addr = addr_info[0][-1]
        mqtt_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        mqtt_socket.settimeout(0.5)
        mqtt_socket.connect(addr)
        self.socket = mqtt_socket
    def time_ms(self):
        return time.time()
    def read(self, length):
        return self.socket.recv(length)
    def write (self, packet):
        self.socket.send(packet)
    def on_message (self, message):
        print(message)

network = NetworkLocal()
mqttclient = MQTTClient(network)
mqttclient.connect()
mqttclient.subscribe("fuga")
def handle_message (message):
    print("on_message")
    print(message)
mqttclient.set_on_message(handle_message)

count = 0
while True:
    mqttclient.loop()
    time.sleep(1.0)
    if count % 5 == 2:
        # pass
        mqttclient.publish("hoge", '{"msg":"hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge hoge punipuni"}', qos=0)
        # mqttclient.publish("hoge", 'fuga')
    count += 1