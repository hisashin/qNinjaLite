import time
import network
from wifi_config import preferred_aps, network_state
import os

print("HEATER ZERO")
from machine import Pin
Pin(25, Pin.OUT).value(0) # Well heater off

wlan = network.WLAN(network.STA_IF)
network_state["wlan"] = wlan
wlan.active(True)

connection_interval = 0.1
connection_timeout = 5.0


def try_connection (target, config):
    delay = 0
    print("Trying to connect...")
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


def connect ():
    # Scan
    print("Scanning...")
    aps = wlan.scan()
    # (ssid, bssid, channel, RSSI, security, hidden)
    # Sort by RSSI ([3])
    sorted_ap = sorted(aps, key=lambda x: x[3], reverse=True)
    for found in sorted_ap:
        for preferred in preferred_aps:
            # Compare
            if (preferred[0] == found[0].decode("utf-8") ):
                result = try_connection(found, preferred)
                if result is not None:
                    print("Connected.")
                    print(result)
                    return

connect()

if "conf" not in os.listdir():
    print("Creating conf dir")
    os.mkdir("conf")
if "certs" not in os.listdir():
    print("Creating certs dir")
    os.mkdir("certs")

try:
    import hardware_batch4
except :
    print("Import error")