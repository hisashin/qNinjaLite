import time
REG_ADDR_PU_CTRL = 0x00
REG_ADDR_CTRL1 = 0x01
REG_ADDR_CTRL2 = 0x02
REG_ADDR_REVISION_CODE = 0x1F

REG_ADDR_ADCO_B2 = 0x12
REG_ADDR_ADCO_B1 = 0x13
REG_ADDR_ADCO_B0 = 0x14

# REG0x00:PU_CTRL
REG_MASK_CYCLE_READY = 0b00100000 # read only (1=ready, 0=not ready)
REG_MASK_CYCLE_START = 0b00010000 # write 1 to start conversion cycle
CYCLE_START = 0b00010000
REG_MASK_RESET_REG = 0b00000001
REG_MASK_POWER_UP_ANALOG = 0b00000100
POWER_UP_ANALOG = 0b00000100
REG_MASK_POWER_UP_DIGITAL = 0b00000010
POEWR_UP_DIGITAL = 0b00000010

# REG0x02:CTRL2
REG_MASK_ANALOG_INPUT_CHANNEL = 0b10000000 # 7 CHS
ANALOG_INPUT_CHANNEL_CH2 = 0b1 << 7
ANALOG_INPUT_CHANNEL_CH1 = 0b0 << 7
REG_MASK_CONVERSION_RATE = 0b01110000 # [6:4] CRS
CONVERSION_RATE_320SPS = 0b111 << 4
CONVERSION_RATE_80SPS = 0b011 << 4
CONVERSION_RATE_40SPS = 0b010 << 4
CONVERSION_RATE_20SPS = 0b001 << 4
CONVERSION_RATE_10SPS = 0b000 << 4

class NAU7802: 
    def __init__(self, i2c, drdy, device_address):
        self.i2c = i2c
        self.drdy = drdy
        self.device_address = device_address
        self.data_rate = 10

    # [read] addr / reg_addr / data (1 or more) stop
    # Read a single value
    def read_reg (self, reg_address):
        self.i2c.writeto(self.device_address, bytearray([reg_address]), False)
        reg = self.i2c.readfrom(self.device_address, 1)
        return reg[0]
    
    # Read mnultiple values with burst mode
    def read_reg_burst (self, reg_address, length):
        self.i2c.writeto(self.device_address, bytearray([reg_address]), False)
        reg = self.i2c.readfrom(self.device_address, length)
        return reg

    # [write] addr / reg_addr / (repeat start) / addr / data (1 or more) / stop
    def write_reg (self, reg_address, value):
        self.i2c.writeto(self.device_address, bytearray([reg_address, value]), False)
        reg = self.i2c.readfrom(self.device_address, 1)

    def select_analog_input_channel (self, channel):
        val = 0
        if channel == 1:
            val = ANALOG_INPUT_CHANNEL_CH1
        elif channel == 2:
            val = ANALOG_INPUT_CHANNEL_CH2
        else:
            raise("Invalid analog input channel. Valid channels are 1 and 2.")
        self.update_reg(REG_ADDR_CTRL2, val, REG_MASK_ANALOG_INPUT_CHANNEL)

    def select_conversion_rate (self, data_rate):
        val = 0
        if data_rate == 330:
            val = CONVERSION_RATE_320SPS
        elif data_rate == 80:
            val = CONVERSION_RATE_80SPS
        elif data_rate == 40:
            val = CONVERSION_RATE_40SPS
        elif data_rate == 20:
            val = CONVERSION_RATE_20SPS
        elif data_rate == 10:
            val = CONVERSION_RATE_10SPS
        else:
            raise("Invalid conversion rate. Valid values are 10, 20, 40, 80 and320.")
        self.data_rate = data_rate
        self.update_reg(REG_ADDR_CTRL2, val, REG_MASK_CONVERSION_RATE)

    def update_reg (self, reg_address, value, mask):
        current_val = self.reg_address(reg_address)
        new_val = (~mask & current_val) | (mask & value)
        self.i2c.writeto(self.device_address, bytearray([reg_address, new_val]))

    def read_conversion_data (self):
        self.update_reg(REG_ADDR_PU_CTRL, CYCLE_START, REG_MASK_CYCLE_START)
        while self.read_reg(REG_ADDR_PU_CTRL) & REG_MASK_CYCLE_READY == 0:
            time.sleep_ms(1)
        #[23:16][15:8][7:0]
        data = self.read_reg_burst(REG_ADDR_ADCO_B2, 3)
        val = (data[0] << 16) | (data[1] << 8) | data[2]
        if val & 0x800000:
            val = ~val + 1
            val = -(val & 0xFFFFFF)
        return val / (1.0 * 0x800000) # -1 to 1

    def reset (self):
        self.update_reg(REG_ADDR_PU_CTRL, 1, REG_MASK_RESET_REG)
        self.update_reg(REG_ADDR_PU_CTRL, 0, REG_MASK_RESET_REG)
        time.sleep_ms(1)

    def start (self):
        self.reset()
        self.update_reg(REG_ADDR_PU_CTRL, POEWR_UP_DIGITAL, REG_MASK_POWER_UP_DIGITAL)
        self.update_reg(REG_ADDR_PU_CTRL, POWER_UP_ANALOG, REG_MASK_POWER_UP_ANALOG)
        time.sleep_ms(1)