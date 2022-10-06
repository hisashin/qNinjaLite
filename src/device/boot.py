# This file is executed on every boot (including wake-boot from deepsleep)
import esp
esp.osdebug(None)
print("boot.py")
import webrepl
webrepl.start()