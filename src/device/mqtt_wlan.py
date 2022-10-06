import time
from mqtt import MQTTClient
import network
import socket
import ssl

server = '192.168.100.59'
port = 1883


class NetworkAWS:
    def __init__(self):
        addr_info = socket.getaddrinfo(server, port)
        print(addr_info)
        addr = addr_info[0][-1]

        # Init SSL
        skt = socket.socket()
        # s = skt
        # s = ssl.wrap_socket(skt, keyfile="cert/private.pem.key", ca_certs="cert/certificate.pem.crt", certfile="cert/CA1.cer")
        print("Connecting...")
        skt.connect(addr)
        print("Connected")
        self.socket = skt
        skt.setblocking(False)
    def time_ms(self):
        return time.ticks_ms()
    def read(self, length):
        # return self.socket.read(length)
        return self.socket.read(length)
    def write (self, packet):
        self.socket.write(packet)

network = NetworkAWS()
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
    if count % 5 == 0:
        # pass
        print("Publishing.")
        mqttclient.publish("hoge", '{"msg":"moge"}', qos=0)
    count += 1