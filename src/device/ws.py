import time
import network
import websocket_helper
import websocket
import socket
from wifi_config import preferred_aps
import select

wlan = network.WLAN(network.STA_IF)
wlan.active(True)

connection_interval = 0.1
connection_timeout = 5.0

class WebSocketServer:
    def __init__(self) -> None:
        self.ws = None
    def start(self, port=8888):
        time_zero = time.ticks_ms()
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        num = 0
        s.bind(("", port))
        s.setblocking(False)
        s.listen(4)
        self.s = s
        print("WebSocketServer started.")
    def close (self):
        self.s.close()
    # Process incoming connections and messages
    def read (self):
        if self.ws == None:
            try:
                conn, addr = self.s.accept() # Raises exception in case of timeout
                print("Connection from %s" % str(addr))
                websocket_helper.server_handshake(conn)
                print("Handshake from %s" % str(addr)) # OK!
                self.ws = websocket.websocket(conn, True)
                conn.setblocking(False)
            except:
                return None

        try:
            msg = self.ws.read()
            if msg is not None:
                return msg.decode("utf-8")
        except:
            return None

    def send (self, message):
        if self.ws is None:
            return
        try:
            self.ws.write(message)
        except:
            print("Disconnected.")
            self.ws = None

if False:
    server = WebSocketServer()
    server.start()

    while True:
        time.sleep(1.0)
        msg = server.read()
        if msg != None:
            print(msg)
        time.sleep(1.0)
        server.send("Hello")
    