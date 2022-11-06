import time

class TLC5929:
    def __init__(self, spi, latch, blank):
        self.spi = spi
        self.latch = latch
        self.blank = blank

    def _latch (self):
        self.latch.on()
        # time.sleep(0.0001)
        self.latch.off()

    def _read (self):
        time.sleep(1)
        self.latch.on()
        time.sleep(0.01)
        self.latch.off()
        time.sleep(0.01)
        rxdata = bytearray(2)
        self.spi.readinto(rxdata, 0x00) 

    def _send (self, msg):
        # time.sleep(0.025)
        self._latch()
        self.spi.write(msg)
        self._latch()
        # time.sleep(0.025)

    def set_brightness (self, brightness) :
        self._send(bytearray([0x01, 0x00, 0xFF & brightness]))

    def select_led (self, selected_led):
        vl = 0x00
        vh = 0x00
        if selected_led < 8:
            vl = 0xFF & ( 1 << selected_led )
        else:
            vh = 0xFF & ( 1 << (selected_led - 8) )
        self._send(bytearray([0x00, vh, vl]))

    def select_all (self):
        self._send(bytearray([0x00, 0xFF, 0xFF]))

    def on (self) :
        self.blank.off()

    def off (self) :
        self.blank.on()
