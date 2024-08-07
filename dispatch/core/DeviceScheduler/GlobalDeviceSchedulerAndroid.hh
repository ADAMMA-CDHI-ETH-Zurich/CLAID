#pragma once

#include "dispatch/core/DeviceScheduler/GlobalDeviceScheduler.hh"
#include "dispatch/core/module_table.hh"
#include "dispatch/core/Module/TypeMapping/TypeMapping.hh"

#include "dispatch/core/RemoteFunction/RemoteFunction.hh"
#include "dispatch/core/RemoteFunction/RemoteFunctionHandler.hh"

using namespace claidservice;

namespace claid {

class GlobalDeviceSchedulerAndroid : public GlobalDeviceScheduler
{
private:
    RemoteFunctionHandler& remoteFunctionHandler;
    RemoteFunction<void> androidAcquireWakeLock;
    RemoteFunction<void> androidReleaseWakeLock;
    RemoteFunction<void> androidScheduleDeviceWakeup;
     
    bool assertJavaRuntimeConnected()
    {
        if(!moduleTable.isRuntimeConnected(Runtime::RUNTIME_JAVA))
        {
            Logger::logError("GlobalDeviceSchedulerAndroid::assertJavaRuntimeConnected: Java runtime is not connected! Cannot use any scheduling functions!");
            return false;
        }
        return true;
    }


public:
    GlobalDeviceSchedulerAndroid(
        RemoteFunctionRunnableHandler& remoteFunctionRunnableHandler, 
        const ModuleTable& moduleTable,
        RemoteFunctionHandler& remoteFunctionHandler
    ) : GlobalDeviceScheduler(remoteFunctionRunnableHandler, moduleTable), remoteFunctionHandler(remoteFunctionHandler)
    {
        androidAcquireWakeLock = remoteFunctionHandler.mapRuntimeFunction<void>(Runtime::RUNTIME_JAVA, "android_acquire_wakelock");
        androidReleaseWakeLock = remoteFunctionHandler.mapRuntimeFunction<void>(Runtime::RUNTIME_JAVA, "android_release_wakelock");
        androidScheduleDeviceWakeup = remoteFunctionHandler.mapRuntimeFunction<void, int64_t>(Runtime::RUNTIME_JAVA, "android_schedule_device_wakeup");
    }

    bool acquirePlatformSpecificWakeLock() override final
    {
        Logger::logInfo("GlobalDeviceSchedulerAndroid (Cpp): acquirePlatformSpecificWakeLock");
        if(!assertJavaRuntimeConnected())
        {
            return false;
        }
        auto result = androidAcquireWakeLock.execute();
        return true;
        // Do not wait here! Will cause deadlock (some runtime requests MIDDLEWARE_CORE::acquireWakeLock, which is processed
        // in Middleware's control package thread. If it then waits here, we can never receive CTRL_REMOTE_FUNCTION_RESPONSE, as response
        // is also processed in Middleware's control package thread).
        // result->await();
        // if(!result->wasExecutedSuccessfully())
        // {
        //     Logger::logError("GlobalDeviceSchedulerAndroid::acquirePlatformSpecificWakeLock: Failed to acquire WakeLock, failed to execute RPC function in Android.");
        // }
    }
    
    bool releasePlatformSpecificWakeLock() override final
    {
        Logger::logInfo("GlobalDeviceSchedulerAndroid (Cpp): releasePlatformSpecificWakeLock");
        if(!assertJavaRuntimeConnected())
        {
            return false;
        }
        auto result = androidReleaseWakeLock.execute();

        // result->await();
        // if(!result->wasExecutedSuccessfully())
        // {
        //     Logger::logError("GlobalDeviceSchedulerAndroid::releasePlatformSpecificWakeLock: Failed to release WakeLock, failed to execute RPC function in Android.");
        // }
        return true;
    }
    
    bool schedulePlatformSpecificDeviceWakeup(int64_t timestamp) override final
    {
        Logger::logInfo("GlobalDeviceSchedulerAndroid (Cpp): schedulePlatformSpecificDeviceWakeup");
        if(!assertJavaRuntimeConnected())
        {
            return false;
        }
        auto result = androidScheduleDeviceWakeup.execute(timestamp);
        return true;
        // result->await();
        // if(!result->wasExecutedSuccessfully())
        // {
        //     Logger::logError("GlobalDeviceSchedulerAndroid::schedulePlatformSpecificDeviceWakeup: Failed to schedule device wakeup, failed to execute RPC function in Android.");
        // }
    }
    

};

}