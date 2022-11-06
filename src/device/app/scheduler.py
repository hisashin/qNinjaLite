from machine import Timer

# Schedule tasks on main thred

class Scheduler ():
    def __init__(self):
        self.schedules = []
        self.max_timer_id = 0
    def add_schedule (self):
        schedule = Schedule(Timer(self.max_timer_id))
        self.schedules.append(schedule)
        self.max_timer_id += 1
        return schedule
    def loop (self):
        for schedule in self.schedules:
            if schedule.has_task:
                schedule.exec_task()

class Schedule ():
    def __init__(self, timer):
        self.timer = timer
        self.has_task = False
    def init_timer(self, period, mode, main_callback):
        self.main_callback = main_callback
        self.timer.init(period=period, mode=mode, callback=self._cb)
    def _cb(self, timer):
        self.has_task = True
    def exec_task(self):
        self.has_task = False
        self.main_callback()
    def cancel_timer(self):
        self.has_task = False
        self.timer.deinit()
