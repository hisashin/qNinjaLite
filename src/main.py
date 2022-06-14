import time
import network
from wifi_config import preferred_aps

wlan = network.WLAN(network.STA_IF)
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
    for preferred in preferred_aps:
        for found in aps:
            # Compare
            if (preferred[0] == found[0].decode("utf-8") ):
                result = try_connection(found, preferred)
                if result is not None:
                    print("Connected.")
                    print(result)
                    return

# wlan.disconnect()
connect()