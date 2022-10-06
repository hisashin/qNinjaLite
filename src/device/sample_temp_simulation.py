# Sample temp simulation
default_temp = 25
class TempSimulation:
    def __init__ (self, temp_air, temp_well):
        # Coeffs (Considering capacity & thermal resistance)
        self.coeff_air = 0.1
        self.coeff_well = 1.2
        # Actual measurement
        self.temp_air = temp_air
        self.temp_well = temp_well
        # Estimated value
        self.temp_sample = (self.coeff_air * self.temp_air + self.coeff_well * self.temp_well) / (self.coeff_air + self.coeff_well)

    def update (self, temp_air, temp_well):
        self.temp_air = temp_air
        self.temp_well = temp_well

    def simulate (self, interval):
        self.temp_sample += ((self.temp_air - self.temp_sample) * self.coeff_air 
            + (self.temp_well - self.temp_sample)  * self.coeff_well) * interval
        return self.temp_sample
