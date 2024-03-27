package adamma.c4dhi.claid.demo_app;

import android.app.Application;

import adamma.c4dhi.claid_platform_impl.CLAID;
import adamma.c4dhi.claid_platform_impl.PersistentModuleFactory;

public class MyApplication extends Application
{
    public PersistentModuleFactory factory;

    @Override
    public void onCreate() {
        super.onCreate();
        factory = CLAID.getPersistentModuleFactory(this);
    }
}
