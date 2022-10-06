# ADS1219IPWR (4ch ADC)

import time
# Datasheet: https:#www.ti.com/store/ti/en/p/product/?p=ADS1219IPWR
# PDF: https:#www.ti.com/lit/ds/symlink/ads1219.pdf?&ts=1589885438370
# Voltage->Value -FS->800000h, 0->000000h, FS->7FFFFFh

# [7:5] MUX
MASK_MUX = 0b11100000
MUX_SINGLE_END_A0 = 0b01100000
MUX_SINGLE_END_A1 = 0b10000000
MUX_SINGLE_END_A2 = 0b10100000
MUX_SINGLE_END_A3 = 0b11000000
MUX_SINGLE_END_VALS = [MUX_SINGLE_END_A0,MUX_SINGLE_END_A1,MUX_SINGLE_END_A2,MUX_SINGLE_END_A3]
MUX_DIFF_PA0_NA1 = 0b00000000
MUX_DIFF_PA2_NA3 = 0b00100000
MUX_DIFF_PA1_NA2 = 0b01000000

# 4 GAIN
MASK_GAIN = 0b00010000
GAIN_1 = 0b00000
GAIN_4 = 0b10000

# [3:2] DR
MASK_DATA_RATE = 0b00001100
DATA_RATE_20SPS = 0b0000
DATA_RATE_90SPS = 0b0100
DATA_RATE_330SPS = 0b1000
DATA_RATE_1000SPS = 0b1100

# 1 CM
MASK_CONVERSION_MODE = 0b00000010
CONVERSION_MODE_SINGLE_SHOT = 0b00
CONVERSION_MODE_CONTINUOUS = 0b10

# 0 VREF
MASK_VREF = 0b00000001
VREF_EXTERNAL = 0b1
VREF_INTERNAL = 0b0 # 2.048V

MASK_DRDY = 0b10000000

# Command
# RESET: Reset 0000 011x
# START/SYNC: Start/Sync 0000 100x
# POWERDOWN: Power down 0000 001x
# RDATA: Read data 0001 xxxx
# RREG: Read register at addr r 0010 0rxx
# WREG: Write configuration register 0100 00xx
COMMAND_RDATA = 0b00010000
COMMAND_SSYNC = 0b00001000
COMMAND_RREG = 0b00100000
COMMAND_RSTAT = 0b00100100
COMMAND_WREG = 0b01000000
COMMAND_RESET = 0b00000110
COMMAND_POWERDOWN = 0b00000010


class ADS1219:
    def __init__(self, i2c, device_address, drdy=None):
        self.i2c = i2c
        self.drdy = drdy
        self.device_address = device_address
        self.data_rate = 330

    def _read_conf_reg (self) :
        # RREG: Read register at addr r 0010 0rxx
        self.i2c.writeto(self.device_address, bytearray([COMMAND_RREG]), False)
        reg = self.i2c.readfrom(self.device_address, 1)
        return reg[0]

    def _read_status_reg (self) :
        # RREG: Read register at addr r 0010 0rxx
        self.i2c.writeto(self.device_address, bytearray([COMMAND_RSTAT]), False)
        reg = self.i2c.readfrom(self.device_address, 1)
        return reg[0]
    
    def _update_reg_value (self, value, mask) :
        current_val = self._read_conf_reg()
        val = (~mask & current_val) | (mask & value)
        self.i2c.writeto(self.device_address, bytearray([COMMAND_WREG, val]))
        self.sync()

    def reset (self):
        self.i2c.writeto(self.device_address, bytearray([COMMAND_RESET]))
        return

    def sync (self):
        self.i2c.writeto(self.device_address, bytearray([COMMAND_SSYNC]))

    def read_conversion_data (self):
        self.sync()
        time.sleep(1.0/self.data_rate)
        wait_count = 0
        if self.drdy:
            while (self.drdy.value()==1):
                wait_count += 1
                if wait_count > 1000:
                    raise Exception("DRDY timeout.")
                time.sleep_ms(1)
        else:
            # Ready DRDY bit instead of physical pin
            while self._read_status_reg & MASK_DRDY == 0:
                wait_count += 1
                if wait_count > 1000:
                    raise Exception("DRDY timeout.")
                time.sleep_ms(1)
        # Issue RDATA command
        self.i2c.writeto(self.device_address, bytearray([COMMAND_RDATA]), False)
        # Read 3 bytes
        dat = self.i2c.readfrom(self.device_address, 3)
        val = 0xFF & dat[0]
        val <<= 8
        val |= (0xFF & dat[1])
        val <<= 8
        val |= (0xFF & dat[2])

        # Negative value
        if(val & 0x00800000):
            val = ~val + 1
            val = -(val & 0xFFFFFF)
        return val / (1.0 * 0x00800000)

    def select_diff_channels (self, channel_p, channel_n) :
        val = 0
        if channel_p == 0 and channel_n == 1:
            val = MUX_DIFF_PA0_NA1
        elif channel_p == 1 and channel_n == 2:
            val = MUX_DIFF_PA1_NA2
        elif channel_p == 2 and channel_n == 3:
            val = MUX_DIFF_PA2_NA3
        else:
            raise Exception("Invalid combination of P and N.")
        self._update_reg_value(val, MASK_MUX)

    # Measure single-ended signals
    def select_single_end_channel (self, channel):
        muxBits = MUX_SINGLE_END_VALS[channel]
        self._update_reg_value(muxBits, MASK_MUX)

    def select_gain (self, gain):
        self._update_reg_value(gain, MASK_GAIN)

    def select_data_rate (self, rate):
        rateBits = DATA_RATE_20SPS
        if (rate == 20):
            rateBits = DATA_RATE_20SPS
        elif (rate == 90) :
            rateBits = DATA_RATE_90SPS
        elif (rate == 330) :
            rateBits = DATA_RATE_330SPS
        elif (rate == 1000) :
            rateBits = DATA_RATE_1000SPS
        else :
            raise Exception("Data rate is invalid. Supported values are 20, 90, 330 and 1000.")
        self.data_rate = rate
        self._update_reg_value(rateBits, MASK_DATA_RATE)

    def select_conversion_mode (self, mode) :
        self._update_reg_value(mode, MASK_CONVERSION_MODE)

    def select_ref_source (self, ref_source):
        self._update_reg_value(ref_source, MASK_VREF)

    def start (self):
        self.reset()
        print("Default reg")
        self.select_data_rate(330)
        self.select_ref_source(VREF_EXTERNAL)
        self.select_conversion_mode(CONVERSION_MODE_CONTINUOUS)
        print("New Reg")
        print(self._read_conf_reg())

