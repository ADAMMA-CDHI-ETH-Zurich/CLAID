/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/
#pragma once
#include <map>
#include "dispatch/proto/claidservice.grpc.pb.h"
#include "dispatch/core/Utilities/Time.hh"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/RemoteFunction/RemoteFunctionRunnableHandler.hh"

namespace claid {

/// The GlobalDeviceScheduler is a central component of the CLAID middleware.
/// It is responsible for scheduling device wakeups and to keep the device awake while
/// any Runnable in any Runtime is still being executed. 
/// The GlobalDeviceScheduler works closely together with the RunnableDispatcher's in each Runtime.
/// The RunnableDispatcher's can directly call functions of the GlobalDeviceScheduler via the Middleware.
/// In fact, the GlobalDeviceScheduler *RELIES* on all RunnableDispatcher's to call acquireWakeLock, releaseWakeLock and scheduleDeviceWakup correctly.
/// The GlobalDeviceScheduler has no "watchdog" by itself, and only reschedules when called by a RunnableDispatcher. 
///
/// The GlobalDeviceScheduler has two core functionalities:
/// 1. It allows each RunnableDispatcher in each Runtime to schedule a device wakeup in X milliseconds.
/// 2. It allows each RunnableDispatcher in each Runtime to acquire a WakeLock, essentially keeping the device from sleeping.
///
/// The GlobalDeviceScheduler is essentially relevant for Android/WearOS, and partially iOS devices.
/// On Android/WearOS, the GlobalDeviceScheduler uses Android's (WakeLock)[https://developer.android.com/develop/background-work/background-tasks/scheduling/wakelock]
/// and (AlarmManager)[https://developer.android.com/reference/android/app/AlarmManager] for the scheduling.  
/// Device wakeups are scheduled using the AlarmManager's "setExactAndAllowWhileIdle" to schedule *exact* device wakeups for the *next due runnable*.
///
/// On Linux and macOS, the scheduler does not do anything, as those devices are always-on anyways.
class GlobalDeviceScheduler
{
    private:
        std::map<int64_t, Runtime> scheduledWakeups;
        std::map<Runtime, int64_t> acquiredWakeLocksPerRuntime;

        int numAcquiredRuntimeWakeLocks = 0;

        // If all Runtimes released their wakelocks, then we could potentially go to sleep.
        // However, if the next wakeup is scheduled shortly afterward, then it is not worth it to schedule a wakeup.
        // We can rather stay awake. For example, if the next runtime is due in 30s, then the device can as well stay awake.
        const int64_t REQUIRED_MILLISECONDS_UNTIL_NEXT_SCHEDULED_WAKEUP_TO_GO_TO_SLEEP = 30000;

        // WakeLock which is acquired if numAcquiredRuntimeWakeLocks is > 0.
        // This means that at least one Runtime holds at least one WakeLock.
        bool runtimeWakeLock = false;
        // WakeLock which is held if the scheduler decides that it is not worth to go to sleep,
        // because the next due Runnable is within REQUIRED_MILLISECONDS_UNTIL_NEXT_SCHEDULED_WAKEUP_TO_GO_TO_SLEEP;
        bool schedulerWakeLock = false;

        bool isPlatformSpecificWakeLockHeld = false;

        void increaseAcquiredWakeLocks(int64_t numWakeLocks);
        void decreaseAcquiredWakeLocks(int64_t numWakeLocks);
        void acquireRuntimeWakeLock();
        void releaseRuntimeWakeLock();
        void updateWakeLockState();

        void cleanupScheduledWakeups(const int64_t currentTimestamp);
        void rescheduleNextWakeup();

    protected:
        const ModuleTable& moduleTable;
        RemoteFunctionRunnableHandler& remoteFunctionRunnableHandler;


        virtual bool acquirePlatformSpecificWakeLock() 
        {
            return true;
        }
        
        virtual bool releasePlatformSpecificWakeLock() 
        {
            return true;
        }
        
        virtual bool schedulePlatformSpecificDeviceWakeup(int64_t timestamp)
        {
            return true;
        }
        

    public:
        GlobalDeviceScheduler(RemoteFunctionRunnableHandler& remoteFunctionRunnableHandler, const ModuleTable& moduleTable);
        virtual ~GlobalDeviceScheduler() {}

        void scheduleDeviceWakeupAt(RuntimeType runtime, int64_t unixTimestampMs);
        void acquireWakeLockForRuntime(RuntimeType runtime);
        void releaseWakeLockForRuntime(RuntimeType runtime);
        
        void releaseAllWakeLocksOfRuntime(Runtime runtime);
};
}