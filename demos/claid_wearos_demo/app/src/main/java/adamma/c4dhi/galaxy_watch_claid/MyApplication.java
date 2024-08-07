package adamma.c4dhi.galaxy_watch_claid;

import android.app.Application;

import adamma.c4dhi.claid_android.Configuration.CLAIDPersistanceConfig;
import adamma.c4dhi.claid_android.Configuration.CLAIDSpecialPermissionsConfig;
import adamma.c4dhi.claid_platform_impl.CLAID;
import adamma.c4dhi.claid_platform_impl.PersistentModuleFactory;
import adamma.c4dhi.claid_android.collectors.battery.BatteryCollector;

public class MyApplication extends Application
{
    @Override
    public void onCreate()
    {
        super.onCreate();
        CLAID.startInPersistentService(getApplicationContext(),
                "assets://claid_config.json",
                "smartwatch",
                "device",
                "user",
                CLAIDSpecialPermissionsConfig.almightyCLAID(),
                CLAIDPersistanceConfig.maximumPersistance());

    }
}
