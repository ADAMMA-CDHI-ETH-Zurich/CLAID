###########################################################################
# Copyright (C) 2023 ETH Zurich
# CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
# Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
# Centre for Digital Health Interventions (c4dhi.org)
# 
# Authors: Patrick Langer
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#         http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

import threading
import time
from typing import List
from logger.logger import Logger
from datetime import datetime

class RunnableDispatcher:
    def __init__(self, main_thread_runnables_queue):
        self.scheduled_runnables = []
        self.mutex = threading.Lock()
        self.condition_variable = threading.Condition(self.mutex)
        self.reschedule_required = False
        self.stopped = True
        self.thread = None
        self.main_thread_runnables_queue = main_thread_runnables_queue

    def get_wait_duration_until_next_runnable_is_due(self):
        with self.mutex:
            if not self.scheduled_runnables:
                # Wait forever.
                return float(1000000)

            microseconds_until_next_runnable_is_due = int((
                self.scheduled_runnables[0].schedule.get_execution_time() - datetime.now()
            ).total_seconds()*1000)

            # Make sure we do not return a negative duration.
            return max(0, microseconds_until_next_runnable_is_due)

    def wait_until_runnable_is_due_or_reschedule_is_required(self):
        # If there is no runnable added, this will be infinity.
        # Hence, we will wait forever and wake up if a reschedule is required.
        wait_time_in_seconds = float(self.get_wait_duration_until_next_runnable_is_due() / 1000)

        # The return value of wait_for can be used to determine whether the wait exited because time passed (False),
        # or because the predicate (self.reschedule_required or self.stopped) evaluates to True (True).
        # However, we are not interested in distinguishing the two cases.
        # In any case, when we wake up, we see if we need to execute anything.
        # wait_for will atomically release the mutex and sleep, and will atomically lock the mutex after waiting.
        with self.condition_variable:
            self.condition_variable.wait_for(
                lambda: self.reschedule_required or self.stopped, wait_time_in_seconds
            )

    def process_runnable(self, scheduled_runnable):
        if self.stopped:
            return

        if scheduled_runnable.runnable.is_valid():
            

            # SWitch to the main thread
            self.main_thread_runnables_queue.put(scheduled_runnable)

            if scheduled_runnable.runnable.stop_dispatcher_after_this_runnable:
                # No more runnables will be executed after this one!
                self.stop()
                Logger.log_info("STOPPED DISPATCHER!")
                return

            if scheduled_runnable.schedule.does_runnable_have_to_be_repeated():
                with self.mutex:
                    planned_execution_time = scheduled_runnable.schedule.get_execution_time()
                    scheduled_runnable.schedule.update_execution_time()

                    # planned_execution_time_str = planned_execution_time.strftime(
                    #     "%d.%m.%y %H:%M:%S"
                    # )
                    # current_execution_str = datetime.now().strftime(
                    #     "%d.%m.%y %H:%M:%S"
                    # )

                    # new_execution_time = scheduled_runnable.schedule.get_execution_time()
                    # next_execution_str = new_execution_time.strftime(
                    #     "%d.%m.%y %H:%M:%S"
                    # )

                    # Logger.log_info(
                    #     f"Runnable, scheduled for execution at {planned_execution_time_str}, has been executed at {current_execution_str}, scheduling next execution for {next_execution_str}",
                    # )

                    # Reinsert the runnable with new scheduled execution time.
                    # Note, that the runnable was removed from scheduled_runnables in the get_and_remove_due_runnables() function.
                    self.scheduled_runnables.append(scheduled_runnable)
                    self.scheduled_runnables.sort(
                        key=lambda x: x.schedule.get_execution_time()
                    )

    def process_runnables(self, runnables):
        for idx, runnable in enumerate(runnables):
            self.process_runnable(runnable)

    def get_and_remove_due_runnables(self):
        runnables = []
        now = datetime.now()

        with self.mutex:
            while self.scheduled_runnables:
                due_runnable = self.scheduled_runnables[0]
                if now >= due_runnable.schedule.get_execution_time():
                    runnables.append(due_runnable)
                    self.scheduled_runnables.pop(0)
                else:
                    break

       

        return runnables

    def run_scheduling(self):
        Logger.log_info("Running scheduling")
        while not self.stopped:
            due_runnables = self.get_and_remove_due_runnables()

            while due_runnables:
                self.process_runnables(due_runnables)
                due_runnables = self.get_and_remove_due_runnables()

            with self.mutex:
                self.reschedule_required = False

            self.wait_until_runnable_is_due_or_reschedule_is_required()

        Logger.log_info("RunnableDispatcher shutdown.")

    def start(self):
        Logger.log_info("Python Starting RunnableDispatcher")
        with self.mutex:
            if not self.stopped:
                return False

            self.stopped = False
            Logger.log_info("Python Starting RunnableDispatcher")
            self.thread = threading.Thread(target=self.run_scheduling)
            self.thread.start()
            return True

    def stop(self):
        with self.mutex:
            if self.stopped:
                return False

            self.stopped = True

            self.condition_variable.notify()
        self.thread.join()
        return True

    def add_runnable(self, scheduled_runnable):
        Logger.log_info("Adding runnable 1")
        with self.mutex:
            Logger.log_info("Adding runnable 2")
            self.scheduled_runnables.append(scheduled_runnable)
            self.scheduled_runnables.sort(
                key=lambda x: x.schedule.get_execution_time()
            )
            Logger.log_info(
                f"Added runnable to RunnableDispatcher, total runnables now: {len(self.scheduled_runnables)}")

            self.reschedule_required = True
            self.condition_variable.notify_all()
            # This will lead to a wake up, so we can reschedule.
           