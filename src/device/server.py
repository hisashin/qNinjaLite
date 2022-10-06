# WebSocket Server
import time
import network
from wifi_config import preferred_aps

wlan = network.WLAN(network.STA_IF)
wlan.active(True)

connection_interval = 0.1
connection_timeout = 5.0

import socket

class HTTPServer:
    def __init__(self) -> None:
        pass
    def try_connection (self, target, config):
        delay = 0
        print("Trying to connect.")
        print(target)
        wlan.connect(config[0], config[1]) # SSID, password
        while not wlan.isconnected() :
            time.sleep(connection_interval)
            delay += connection_interval
            if delay > connection_timeout:
                print("Timeout.")
                wlan.disconnect()
                return None # Failed
        # Return wlan config
        return wlan.ifconfig()
    def connect (self):
        wlan.disconnect()
        # Scan
        print("Scanning...")
        aps = wlan.scan()
        for preferred in preferred_aps:
            for found in aps:
                # Compare
                if (preferred[0] == found[0].decode("utf-8") ):
                    result = self.try_connection(found, preferred)
                    if result is not None:
                        print("Connected.")
                        print(result)
                        self.on_connect()
                        return
    def on_connect(self):
        print("on_connect")
        time_zero = time.ticks_ms()
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        num = 0
        s.bind(('', 8080))
        s.setblocking(False)
        s.listen(4)
        while True:
            time.sleep(1.0)
            print("Loop {}".format(time.ticks_ms()-time_zero))
            try:
                conn, addr = s.accept() # Raises exception in case of timeout
                print('Connection from %s' % str(addr))
                request = conn.recv(1024)
                request = str(request)
                print(request)
                message = "uPython server {}".format(num)
                conn.send('HTTP/1.1 200 OK\n')
                conn.send('Content-Type: text/html\n')
                conn.send('Connection: close\n\n')
                conn.send(message)
                conn.close()
                num += 1
            except:
                pass
ws_server = HTTPServer()
ws_server.connect()
