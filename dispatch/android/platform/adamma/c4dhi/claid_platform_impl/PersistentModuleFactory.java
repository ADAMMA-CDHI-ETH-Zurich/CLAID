package adamma.c4dhi.claid_platform_impl;

import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.ModuleFactory;

import android.app.Application;


public class PersistentModuleFactory extends ModuleFactory {

    private Application application;

    public <T extends Application> PersistentModuleFactory(T application) throws IllegalArgumentException
    {
        super();
        if (!(application instanceof Application)) {
            // Handle the case where application is not an instance of Application
            CLAID.onUnrecoverableException("The provided object is not an instance of Application.");
        }

        if (application.getClass().getName().equals(Application.class.getName())) {
            // Handle the case where application is an instance of the standard Application class
            CLAID.onUnrecoverableException("Cannot create PersistentModuleFactory using the standard Application class.\n"
                    + "You need to create your own Application class inheriting from Application in order to create a PersistentModuleFactory.\n"
                    + "You have to populate the PersistentModuleFactory inside your Application's onCreate function for the PersistentModuleFactory to be persistent.");
        }

        this.application = application;
    }

    public Application getApplication()
    {
        return this.application;
    }

   
}
