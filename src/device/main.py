import time
import network
from wifi_config import preferred_aps, network_state
from machine import Pin, SoftI2C
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
                    return True
    return False


if "conf" not in os.listdir():
    print("Creating conf dir")
    os.mkdir("conf")
if "certs" not in os.listdir():
    print("Creating certs dir")
    os.mkdir("certs")


pin_ap_mode=Pin(33, Pin.IN, Pin.PULL_UP)
pin_no_run=Pin(2, Pin.IN, Pin.PULL_UP)
PIN_VALUE_ON = 0

def lcd_message (msg0, msg1):
    import ssd1306 #LCD
    lcd_device_address = 64
    scl = Pin(12, Pin.OUT, Pin.PULL_UP)
    sda = Pin(13, Pin.OUT, Pin.PULL_UP)
    i2c = SoftI2C(scl, sda, freq=80000)
    lcd = ssd1306.SSD1306_I2C(128, lcd_device_address, i2c)
    lcd.fill(1)
    lcd.text(msg0, 2, 2, 0)
    lcd.text(msg1, 2, 16, 0)
    lcd.show()

def ap_mode ():
    ap = network.WLAN(network.AP_IF)
    ap.active(False)
    ap.active(True)
    ap.config(essid="qNinjaLITE")
    lcd_message("AP Mode",ap.ifconfig()[0])

print([pin_ap_mode.value(), pin_no_run.value()])

print("Importing cycler_mqtt")
import cycler_mqtt