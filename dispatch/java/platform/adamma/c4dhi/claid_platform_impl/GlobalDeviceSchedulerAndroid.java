
// Just a stub class as a pendant for the GlobalDeviceScheduler class under Android.
// Does nothing, just implements stubs for the expected functions.
public class GlobalDeviceSchedulerAndroid 
{
    public GlobalDeviceSchedulerAndroid(RemoteFunctionRunnableHandler runnableHandler)
    {

    }

    public GlobalDeviceSchedulerAndroid(RemoteFunctionRunnableHandler runnableHandler)
    {
        runnableHandler.registerRunnable(this, "android_acquire_wakelock", null);
        runnableHandler.registerRunnable(this, "android_release_wakelock", null);
        runnableHandler.registerRunnable(this, "android_schedule_device_wakeup_at", null, Long.class);
    }

    public void android_acquire_wakelock()
    {
        
    }

    public void android_release_wakelock()
    {
        
    }

    public void android_schedule_device_wakeup_at(Long scheduledTimeUnixTimestamp)
    {
        
    }
}
