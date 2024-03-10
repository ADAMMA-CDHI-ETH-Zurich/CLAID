package adamma.c4dhi.claid_android.CLAIDServices;

import android.content.Context;
import android.content.Intent;

import adamma.c4dhi.claid_android.CLAIDServices.MaximumPermissionsPerpetualService;
import adamma.c4dhi.claid_android.CLAIDServices.ServiceRestartDescription;
import adamma.c4dhi.claid_android.CLAIDServices.ServiceRestarterWorker;
import adamma.c4dhi.claid_android.Configuration.CLAIDPersistanceConfig;
import adamma.c4dhi.claid_android.Permissions.Permission;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid_platform_impl.CLAID;

import androidx.work.ExistingPeriodicWorkPolicy;
import androidx.work.PeriodicWorkRequest;
import androidx.work.WorkManager;

import java.io.File;
import java.util.concurrent.TimeUnit;

public class ServiceManager
{
    public static void startMaximumPermissionsPerpetualService(Context context, 
        final String socketPath, final String configFilePath, final String hostId, 
        final String userId, final String deviceId, CLAIDPersistanceConfig persistanceConfig)
    {   
        Permission.setContext(context);
        // Request all permissions.
        if(!MaximumPermissionsPerpetualService.requestRequiredPermissions(context, persistanceConfig))
        {
            Logger.logError("Failed to start CLAIDServices.MaximumPermissionsPerpetualService.\n" + 
                            "Required permissions were not granted.");
        }

        final String restartDescriptionPath = CLAID.getAppDataDirectory(context) + "/" + "claid_service_restart_description.dat";
        ServiceRestartDescription description = new ServiceRestartDescription();

        description.put("serviceType", MaximumPermissionsPerpetualService.class.getName());
        description.put("socketPath", socketPath);
        description.put("configFilePath", configFilePath);
        description.put("hostId", hostId);
        description.put("userId", userId);
        description.put("deviceId", deviceId);
        description.put("startOnBoot", persistanceConfig.RESTART_ON_BOOT ? "true" : "false");
        description.put("restartOnCrashOrTermination", persistanceConfig.MONITOR_TRY_RESTART_IF_CRASHED_OR_EXITED ? "true" : "false");

        if(!description.serializeToFile(restartDescriptionPath))
        {
            CLAID.onUnrecoverableException("Failed to write ServiceRestartDescription to file \"" + restartDescriptionPath + "\".\n" + 
                            "Without the ServiceRestartDescription, the Service is not able to restart on boot or after termination.\n" + 
                            "Failed to start MaximumPermissionsPerpetualService");
        }
        Logger.logInfo("Calling appropriate startup method");

        executeConfiguredStartupMethodForMaximumPermissionsPerpetualService(context);
    }

    public static void executeConfiguredStartupMethodForMaximumPermissionsPerpetualService(Context context)
    {
        Logger.logInfo("Choosing appropriate startup method");
        // Register periodic work task, which acts as a "watchdog", checking if
        // the service is running and if not restarts it.
        // First execution is now (immediately), then every 16 minutes.
        if(shouldRestartMaximumPermissionsPerpetualServiceOnTermination(context))
        {
            Logger.logInfo("Starting CLAIDService by WorkManager watchdog");

            // Start service first, then enqueue periodic task.
            // Periodic task should normally be run immediately, and then every 16 minutes.
            // However, Android can also decide to not run it for the first time.
            // In that case, our Service would only be started after 16 minutes.
            // Hence, we start it first. If the ServiceRestarterWorker kicks in, it will check if the service is running.
            restartMaximumPermissionsPerpetualServiceByIntent(context);

            try{
                Thread.sleep(2000);
            }
            catch(InterruptedException e)
            {
                Logger.logWarning("ServiceManager::executeConfiguredStartupMethodForMaximumPermissionsPerpetualService " + e.getMessage());
                e.printStackTrace();
            }

            WorkManager workManager = WorkManager.getInstance(context);
            String UNIQUE_WORK_NAME = "CLAIDServiceWatchdog";
            // As per Documentation: The minimum repeat interval that can be defined is 15 minutes
            // (same as the JobScheduler API), but in practice 15 doesn't work. Using 16 here
            PeriodicWorkRequest request =
                    new PeriodicWorkRequest.Builder(
                            ServiceRestarterWorker.class,
                            16,
                            TimeUnit.MINUTES)
                            .build();

            // to schedule a unique work, no matter how many times app is opened i.e. startServiceViaWorker gets called
            // do check for AutoStart permission
            Logger.logInfo("Starting CLAIDService enqueing periodic work");

            workManager.enqueueUniquePeriodicWork(UNIQUE_WORK_NAME, ExistingPeriodicWorkPolicy.KEEP, request);
        }
        else
        {
            Logger.logInfo("Starting CLAIDService without watchdog");
            restartMaximumPermissionsPerpetualServiceByIntent(context);
        }
    }

    public static void restartMaximumPermissionsPerpetualServiceByIntent(Context context)
    {
        Logger.logInfo("Restaring service, creating intent.");

        // Check if the ServiceRestartDescription exists.
        // It will be used on the onStartCommand function of the service.
        // If it does not exist here, we don't even have to try launching the service.
        ServiceRestartDescription description = getServiceRestartDescription(context);

        if(description == null)
        {
            CLAID.onUnrecoverableException("Failed restart MaximumPermissionsPerpetualService. Failed to load restart description.");
        }

        Intent serviceIntent = new Intent(context, MaximumPermissionsPerpetualService.class);
        context.startForegroundService(serviceIntent);
        Logger.logInfo("Restaring service, intent launched.");
    }    

    public static ServiceRestartDescription getServiceRestartDescription(Context context)
    {
        ServiceRestartDescription description = new ServiceRestartDescription();
        final String restartDescriptionPath = CLAID.getAppDataDirectory(context) + "/" + "claid_service_restart_description.dat";

        if(!description.deserializeFromFile(restartDescriptionPath))
        {
            return null;
        }
        return description;
    }

    public static void deleteServiceRestartDescription(Context context)
    {
        final String restartDescriptionPath = CLAID.getAppDataDirectory(context) + "/" + "claid_service_restart_description.dat";
        File description = new File(restartDescriptionPath);
        if(description.exists() && description.isFile())
        {
            description.delete();
        }
    }

    public static boolean shouldStartMaximumPermissionsPerpetualServiceOnBoot(Context context)
    {
        ServiceRestartDescription restartDescription = getServiceRestartDescription(context);
        if(restartDescription == null)
        {
            return false;
        }

        return restartDescription.get("startOnBoot").equals("true");
    }

    public static boolean shouldRestartMaximumPermissionsPerpetualServiceOnTermination(Context context)
    {
        ServiceRestartDescription restartDescription = getServiceRestartDescription(context);
        if(restartDescription == null)
        {
            return false;
        }

        return restartDescription.get("restartOnCrashOrTermination").equals("true");
    }
    
    public static boolean isServiceRunning()
    {
        return MaximumPermissionsPerpetualService.isRunning;
    }

    public static void stopService(Context context)
    {
        if(!isServiceRunning())
        {
            return;
        }

        Intent stopServiceIntent = new Intent(context, MaximumPermissionsPerpetualService.class);

        context.stopService(stopServiceIntent);
        deleteServiceRestartDescription(context);
    }
}

