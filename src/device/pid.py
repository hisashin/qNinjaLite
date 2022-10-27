# pid control library

class PIDRange:
    def __init__(self, kp, ki, kd, min_value=None, max_value=None):
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.min_value = min_value
        self.max_value = max_value

CONTROL_TYPE_NORMAL = 1
CONTROL_TYPE_RAMP_INCREASE = 2
CONTROL_TYPE_RAMP_DECREASE = 3
CONTROL_TYPE_OFF = 4
class PID:
    def __init__(self, ranges):
        # ranges should be pre-sorted by values in ascending order
        if not ranges:
            raise("Parameter ranges should be an array with one or more PIDRange objects.")
        self.ranges = ranges
        self.range = None
        
        self.value = 0
        self.prevValue = 0
        self.setpoint = 0
        self.rampSetpoint = 0
        self.output = 0
        self.kp = 0
        self.ki = 0
        self.kd = 0
        self.p = 0
        self.i = 0
        self.d = 0
        self.interval = 1.0
        self.output_lower = -1
        self.output_upper = 1.0
        self.output_off = 0
        self.controlType = CONTROL_TYPE_OFF
        
    def set_interval (self, interval):
        self.interval = interval

    def set_setpoint(self, setpoint):
        if setpoint == None:
            self.controlType = CONTROL_TYPE_RAMP_DECREASE
            return
        if setpoint < self.setpoint - 10:
            self.controlType = CONTROL_TYPE_RAMP_DECREASE
        elif setpoint > self.setpoint + 10:
            self.controlType = CONTROL_TYPE_RAMP_INCREASE
        else:
            self.controlType = CONTROL_TYPE_NORMAL
        self.setpoint = setpoint

    def set_output_range(self, lower, upper, off=0):
        self.output_lower = lower
        self.output_upper = upper
        self.output_off = off
    
    def back_calc_setpoint_diff (self, ctrl):
        return (self.ki*self.i + (self.kd*(self.value-self.prevValue)/self.interval) + ctrl) / (self.kp + self.ki*self.interval);

    def set_value(self, value):
        if self.controlType == CONTROL_TYPE_OFF:
            return
        new_range = self.select_range(value)
        if self.range != new_range:
            self.range = new_range
            self.kp = self.range.kp
            self.ki = self.range.ki
            self.kd = self.range.kd
        # From JS simulator
        self.value = value
        rampSetpoint = self.setpoint
        if self.controlType == CONTROL_TYPE_RAMP_INCREASE:
            ctrl = self.output_upper
            diff = self.back_calc_setpoint_diff(ctrl)
            rampSetpoint = min(self.setpoint, self.value+diff)
            if self.setpoint <= self.value:
                self.controlType = CONTROL_TYPE_NORMAL
        if self.controlType == CONTROL_TYPE_RAMP_DECREASE:
            ctrl = self.output_lower
            diff = self.back_calc_setpoint_diff(ctrl)
            rampSetpoint = max(self.setpoint, self.value+diff)
            if self.setpoint >= self.value:
                self.controlType == CONTROL_TYPE_NORMAL
        self.p = self.value - rampSetpoint
        self.i += (self.value - rampSetpoint) * self.interval
        self.d = (self.value - self.prevValue) / self.interval

        print("V=%.2f p=%.2f(%.2f)\ti=%.2f(%.2f)\td=%.2f(%.2f) SP=%.1f" % (self.value, self.p, self.p*self.kp, self.i, self.i*self.ki, self.d, self.d*self.kd, rampSetpoint))
        self.prevValue = self.value

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
        if self.controlType == CONTROL_TYPE_OFF:
            self.output = self.output_off
        else:
            desiredOutput = - (self.p * self.kp + self.i * self.ki + self.d * self.kd)
            self.output = max(min(self.output_upper, desiredOutput), self.output_lower)
        return self.output

    def reset(self):
        print("pid reset")
        self.p = 0
        self.i = 0
        self.d = 0