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
import asyncio

class RunnableDispatcher:
    def __init__(self):
        # Data structures
        self.scheduled_runnables = []

        # Lock & condition for asyncio
        self.lock = asyncio.Lock()
        # Workaround for issue #358: https://github.com/aio-libs/janus/issues/358
        getattr(self.lock, '_get_loop', lambda: None)()
        self.condition = asyncio.Condition(self.lock)

        self.reschedule_required = False
        self.stopped = True

        # Instead of a Thread, we'll store an asyncio Task
        self._scheduler_task = None


    async def start(self):
        """Start the dispatcher as an async Task."""
        async with self.lock:
            if not self.stopped:
                return False
            self.stopped = False

        # Create and schedule the run_scheduling() task
        self._scheduler_task = asyncio.create_task(self.run_scheduling())
        return True

    async def stop(self):
        """Stop the dispatcher and wait for the scheduling task to finish."""
        async with self.lock:
            if self.stopped:
                return False
            self.stopped = True
            # Wake up the run_scheduling() loop
            self.condition.notify_all()

        # Wait for the scheduling task to exit
        if self._scheduler_task:
            await self._scheduler_task

        return True

    async def add_runnable(self, scheduled_runnable):
        """Add a runnable and notify the dispatcher to possibly reschedule."""
        Logger.log_info("add runnable 1")
        async with self.lock:
            Logger.log_info("add runnable 2")
            self.scheduled_runnables.append(scheduled_runnable)
            self.scheduled_runnables.sort(key=lambda x: x.schedule.get_execution_time())
            Logger.log_info(
                f"Added runnable, total runnables now: {len(self.scheduled_runnables)}"
            )
            Logger.log_info("add runnable 3")

            self.reschedule_required = True
            Logger.log_info(f"{self} Notified")
            self.condition.notify_all()  # Wake the loop so it can reschedule

    def get_wait_duration_until_next_runnable_is_due(self):
        """
        Computes how long (in milliseconds) until the next runnable is due.
        This is a synchronous check of self.scheduled_runnables,
        but it must be called while the lock is held.
        """
        if not self.scheduled_runnables:
            # Nothing is scheduled, so wait a very long time.
            return 1_000_000.0  # effectively "forever"

        next_due_time = self.scheduled_runnables[0].schedule.get_execution_time()
        diff_seconds = (next_due_time - datetime.now()).total_seconds()
        milliseconds_until_due = diff_seconds * 1000.0
        return max(0.0, milliseconds_until_due)  # No negative durations

    async def _wait_for_reschedule(self):
        """Wait until `self.reschedule_required` becomes True."""
        async with self.condition:
            while not self.reschedule_required and not self.stopped:
                Logger.log_info("Waiting")
                await self.condition.wait()  # Wait for notification
                Logger.log_info(f"Waiting done {self.reschedule_required} {self.stopped}")

    async def wait_until_runnable_is_due_or_reschedule_is_required(self):
        """
        Wait until either:
          - The next runnable is due (timeout expires), or
          - self.reschedule_required or self.stopped is set to True.
        """
        Logger.log_info("Getting wait time")
        async with self.lock:
            # Compute how long to wait (in seconds for asyncio)
            wait_time_in_seconds = self.get_wait_duration_until_next_runnable_is_due() / 1000.0
        Logger.log_info("Getting wait time2")

        # Wait for either the predicate or a timeout.
        # Note: async with self.condition *again*, because wait_for needs the condition locked.
        try:
            Logger.log_info("Getting wait time")

          
            Logger.log_info("Getting wait time 2")
            await asyncio.wait_for(
                self._wait_for_reschedule(),
                timeout=wait_time_in_seconds
            )
            Logger.log_info("Condition woke up")
        except asyncio.TimeoutError:
            # A TimeoutError just means a new runnable was added
            # or we waited for timeout. We can safely ignore the "error".
            Logger.log_info("Condition woke up")
            pass

    async def run_scheduling(self):
        """The main scheduling loop that runs until stopped."""
        Logger.log_info("Running scheduling")
        while True:
            async with self.lock:
                if self.stopped:
                    break

            # Gather runnables that are due NOW
            due_runnables = await self.get_and_remove_due_runnables()
            while due_runnables:
                # Process them
                await self.process_runnables(due_runnables)
                # Check if more became due while we were processing
                due_runnables = await self.get_and_remove_due_runnables()
            Logger.log_info("done processing")
            # Reset reschedule_required before we wait again
            async with self.lock:
                self.reschedule_required = False

            # Now wait for the next due runnable or a reschedule
            await self.wait_until_runnable_is_due_or_reschedule_is_required()

        Logger.log_info("RunnableDispatcher shutdown.")

    async def get_and_remove_due_runnables(self):
        """Return and remove all runnables that are currently due."""
        async with self.lock:
            runnables = []
            now = datetime.now()
            while self.scheduled_runnables:
                due_runnable = self.scheduled_runnables[0]
                if now >= due_runnable.schedule.get_execution_time():
                    runnables.append(due_runnable)
                    self.scheduled_runnables.pop(0)
                else:
                    break
            return runnables

    async def process_runnables(self, runnables):
        """Process each runnable in turn."""
        Logger.log_info(f"Processing runnable {runnables}")
        for r in runnables:
            await self.process_runnable(r)

    async def process_runnable(self, scheduled_runnable):
        """Process a single runnable."""
        Logger.log_info("Process runnable 1")
        # It's possible that we've been stopped in the interim:
        async with self.lock:
            if self.stopped:
                return
        Logger.log_info("Process runnable 2")

        if scheduled_runnable.runnable.is_valid():
            Logger.log_info("Process runnable 3")
            # Switch to the "main thread" or some main queue for the actual run
            await scheduled_runnable.runnable.run()
            Logger.log_info("Process runnable 4")

            if scheduled_runnable.runnable.stop_dispatcher_after_this_runnable:
                await self.stop()
                return

            # If it must be repeated, schedule it again.
            if scheduled_runnable.schedule.does_runnable_have_to_be_repeated():
                # Recalculate the planned execution time
                scheduled_runnable.schedule.update_execution_time()
                async with self.lock:
                    self.scheduled_runnables.append(scheduled_runnable)
                    self.scheduled_runnables.sort(
                        key=lambda x: x.schedule.get_execution_time()
                    )
           