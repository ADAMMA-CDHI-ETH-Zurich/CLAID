from module.scheduling.schedule_description import ScheduleDescription

from module.scheduling.scheduled_runnable import ScheduledRunnable


# A runnable wrapper to schedule runnables from outside of CLAID.
# Only works with the CLAID main_thread_queue.
class ExternalScheduledRunnable(ScheduledRunnable):
    def __init__(self, runnable = None, schedule = None):
        self.runnable = runnable
        self.schedule = schedule

    def set_runnable(self, runnable):
        self.runnable = runnable

    def set_schedule(self, schedule: ScheduleDescription):
        self.schedule = schedule

