#pragma once

#include "dispatch/core/DeviceScheduler/GlobalDeviceScheduler.hh"
#include "dispatch/core/module_table.hh"
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
        RemoteFunctionHandler& remoteFunctionHandler, 
        const ModuleTable& moduleTable
    ) : GlobalDeviceScheduler(moduleTable), remoteFunctionHandler(remoteFunctionHandler)
    {
        androidAcquireWakeLock = remoteFunctionHandler.mapRuntimeFunction<void>(Runtime::RUNTIME_JAVA, "acquire_wake_lock");
        androidReleaseWakeLock = remoteFunctionHandler.mapRuntimeFunction<void>(Runtime::RUNTIME_JAVA, "release_wake_lock");
        androidScheduleDeviceWakeup = remoteFunctionHandler.mapRuntimeFunction<void, int64_t>(Runtime::RUNTIME_JAVA, "schedule_device_wakeup");
    }

    void acquirePlatformSpecificWakeLock() override final
    {
        if(!assertJavaRuntimeConnected())
        {
            return;
        }
        androidAcquireWakeLock.execute();
    }
    
    void releasePlatformSpecificWakeLock() override final
    {
        if(!assertJavaRuntimeConnected())
        {
            return;
        }
        androidReleaseWakeLock.execute();
    }
    
    void schedulePlatformSpecificDeviceWakeup(const int64_t timestamp) override final
    {
        if(!assertJavaRuntimeConnected())
        {
            return;
        }
        androidScheduleDeviceWakeup.execute(timestamp);
    }
    

}

};