# pid control library

class PIDRange:
    def __init__(self, kp, ki, kd, min_value, max_value):
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.min_value = min_value
        self.max_value = max_value

class PID:
    def __init__(self, ranges):
        # ranges should be pre-sorted by values in ascending order
        if not ranges:
            raise("Parameter ranges should be an array with one or more PIDRange objects.")
        self.ranges = ranges
        self.value = 0
        self.setpoint = 0
        self.range = None
        self.kp = 0
        self.ki = 0
        self.kd = 0
        self.p = 0
        self.i = 0
        self.d = 0
        self.interval = 1.0
        self.lower = -1
        self.upper = 1.0
        self.output = 0
        
    def set_setpoint(self, setpoint):
        self.setpoint = setpoint

    def set_interval (self, interval):
        self.interval = interval

    def reset(self, value):
        self.p = 0
        self.i = 0
        self.d = 0

    def set_output_range(self, lower, upper):
        self.lower = lower
        self.upper = upper

    def set_value(self, value):
        self.p = value - self.setpoint
        if self.output >= self.upper or self.output <= self.lower:
            self.i = 0
        self.i += (value - self.setpoint) * self.interval
        self.d = (value - self.value) / self.interval
        self.value = value

    def select_range(self, value):
        for range in self.ranges:
            min_flg = range.min_value == None
            if not min_flg:
                min_flg = range.min_value <= value
            max_flg = range.max_value == None
            if not max_flg:
                max_flg =range.max_value >= value
            if max_flg and min_flg:
                return range

    def get_output(self):
        new_range = self.select_range(self.value)
        if self.range != new_range:
            self.range = new_range
            self.kp = self.range.kp
            self.ki = self.range.ki
            self.kd = self.range.kd
        # print(["p,i,d", self.p, self.i, self.d])
        output = - (self.p * self.kp + self.i * self.ki + self.d * self.kd)
        # Clip
        output = min(self.upper, max(self.lower, output))
        self.output = output
        return output

