#include "dispatch/core/DeviceScheduler/GlobalDeviceScheduler.hh"

namespace claid {
    void GlobalDeviceScheduler::increaseAcquiredWakeLocks(int64_t numWakeLocks)
    {
        if(numWakeLocks < 0)
        {
            Logger::logFatal("Fatal error in GlobalDeviceScheduler::increaseAcquiredWakeLocks. "
            "The value of numWakeLocks is %d which isless than 0.", numWakeLocks);
            return;
        }
        if(numAcquiredRuntimeWakeLocks == 0 && numWakeLocks > 0)
        {
            acquireRuntimeWakeLock();
        }
        numAcquiredRuntimeWakeLocks += numWakeLocks;
    }

    void GlobalDeviceScheduler::decreaseAcquiredWakeLocks(int64_t numWakeLocks)
    {
        if(numWakeLocks < 0)
        {
            Logger::logFatal("Fatal error in GlobalDeviceScheduler::decreaseAcquiredWakeLocks. "
            "The value of numWakeLocks is %d which is less than 0.", numWakeLocks);
            return;
        }

        if(numAcquiredRuntimeWakeLocks > 0)
        {
            if(numWakeLocks == numAcquiredRuntimeWakeLocks)
            {
                Logger::logInfo(
                    "GlobalDeviceScheduler::decreaseAcquiredWakeLocks decreasing numAcquiredRuntimeWakeLocks by %d, number of wakelocks is now: %",
                    numAcquiredRuntimeWakeLocks,
                    numWakeLocks - numAcquiredRuntimeWakeLocks);

                releaseRuntimeWakeLock();
            }
            else if(numWakeLocks > numAcquiredRuntimeWakeLocks)
            {
                Logger::logFatal(
                    "Fatal error in GlobalDeviceScheduler::decreaseAcquiredWakeLocks. "
                    "The number of wakelocks is %d, and we shall decrease it by  %d, which would result in a negative amount of WakeLocks."
                    "This is a serious bug.", 
                    numAcquiredRuntimeWakeLocks, 
                    numWakeLocks);
                return;
            }
        }
    }

    void GlobalDeviceScheduler::acquireRuntimeWakeLock()
    {
        runtimeWakeLock = true;
        updateWakeLockState();
    }

    void GlobalDeviceScheduler::releaseRuntimeWakeLock()
    {
        runtimeWakeLock = false;
        // Checks if there are any wakeups planned.
        // If the next wakeup is more than REQUIRED_MILLISECONDS_UNTIL_NEXT_SCHEDULED_WAKEUP_TO_GO_TO_SLEEP away, 
        // then rescheduleNextWakeup will update the current WakeLock state to allow the device to go to sleep.
        rescheduleNextWakeup();
    }

    void GlobalDeviceScheduler::cleanupScheduledWakeups(const uint64_t currentTimestamp)
    {
        Logger::logInfo("Cleaning up scheduled wakeups.");
        // Find the first element with a key not less than the current time;
        auto it = scheduledWakeups.lower_bound(currentTimestamp);
        
        // Erase all elements with scheduled times less than currentTimestamp.
        // Those are outdated.
        scheduledWakeups.erase(scheduledWakeups.begin(), it);
    }

    void GlobalDeviceScheduler::rescheduleNextWakeup()
    {
        const uint64_t currentTimestamp = Time::now().toUnixTimestampMilliseconds();
        
        uint64_t nextScheduledWakeupTime = scheduledWakeups.begin()->first;

        while(nextScheduledWakeupTime < currentTimestamp)
        {
            cleanupScheduledWakeups(currentTimestamp);
            nextScheduledWakeupTime = scheduledWakeups.begin()->first;
        }

        if(runtimeWakeLock)
        {
            Logger::logInfo(
                "GlobalDeviceScheduler::rescheduleNextWakup: Runtime wakelock is still active, thus we do not have to reschedule a device wakeup. "
                "Number of current runtime wakelocks: %d", 
                numAcquiredRuntimeWakeLocks
            );
            return;
        }

        if(scheduledWakeups.size() == 0)
        {
            schedulerWakeLock = false;
            Logger::logWarning(
                "GlobalDeviceScheduler::rescheduleNextWakup: No wakeups scheduled and no wakelock acquired by any runtime! Device is going to sleep, "
                "and might not wake up anytime soon. Device wakeup is now out of CLAID's hands."
            );
        }


        if(currentTimestamp - nextScheduledWakeupTime < REQUIRED_MILLISECONDS_UNTIL_NEXT_SCHEDULED_WAKEUP_TO_GO_TO_SLEEP)
        {
            // The next scheduled wake up is too close.
            // It is not worth to go to sleep.
            // We hold a wakelock instead.
            schedulerWakeLock = true;
            return;
        }
        else
        {
            schedulerWakeLock = false;
            schedulePlatformSpecificDeviceWakeup(currentTimestamp);
        }
        updateWakeLockState();
    }

    /// Judges whether we have to acquire a wakelock or schedule a device wakeup.
    void GlobalDeviceScheduler::updateWakeLockState()
    {
        if(runtimeWakeLock || schedulerWakeLock)
        {
            // We have to hold a wakelock.
            // Check if it is already held, if yes ignore.
            if(!isPlatformSpecificWakeLockHeld)
            {
                Logger::logInfo(
                    "Updating wakelock. Runtimewakelock is %s, schedulerWakeLock is %s"
                    "Acquiring platform specific wakelock.",
                    runtimeWakeLock ? "true" : "false",
                    schedulerWakeLock ? "true" : "false"
                );
                acquirePlatformSpecificWakeLock();
                isPlatformSpecificWakeLockHeld = true;
            }
        }
        else 
        {
            // We do not have to hold a wakelock.
            // Check if it is held and if yes release it.
            if(isPlatformSpecificWakeLockHeld)
            {
                Logger::logInfo(
                    "Updating wakelock. Runtimewakelock is %s, schedulerWakeLock is %s"
                    "Releasing platform specific wakelock.",
                    runtimeWakeLock ? "true" : "false",
                    schedulerWakeLock ? "true" : "false"
                );
                releasePlatformSpecificWakeLock();
                isPlatformSpecificWakeLockHeld = false;
            }
        }
    }

    GlobalDeviceScheduler::GlobalDeviceScheduler(const ModuleTable& moduleTable) : moduleTable(moduleTable)
    {

    }

    void GlobalDeviceScheduler::scheduleDeviceWakeupAt(Runtime runtime, uint64_t unixTimestampMs)
    {
        scheduledWakeups[unixTimestampMs] = runtime;
        rescheduleNextWakeup();
    }

    void GlobalDeviceScheduler::acquireWakeLockForRuntime(Runtime runtime)
    {
        increaseAcquiredWakeLocks(1);
        acquiredWakeLocksPerRuntime[runtime]++;
    }

    void GlobalDeviceScheduler::releaseWakeLockForRuntime(Runtime runtime)
    {
        decreaseAcquiredWakeLocks(1);
        acquiredWakeLocksPerRuntime[runtime]--;
    }

    void GlobalDeviceScheduler::releaseAllWakeLocksOfRuntime(Runtime runtime)
    {
        const int64_t numWakeLocksOfRuntime = acquiredWakeLocksPerRuntime[runtime];
        decreaseAcquiredWakeLocks(numWakeLocksOfRuntime);
        acquiredWakeLocksPerRuntime[runtime] = 0;
    }
}