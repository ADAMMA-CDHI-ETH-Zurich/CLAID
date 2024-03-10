// This class provides CLAID's effort to provide a Service that is able 
// to perform everything in the background (at least everything that Android still allows),
// and is as  persistent as possible on the Android OS, without having root privileges (i.e., without being a system service).
// It runs as long as possible, auto starts on boot, restarts after a crash and ideally even after being terminated by the OS due to resource restrictions.
// Optionally, it can also prevent the device from going to doze or deep sleep by using WakeLocks.
// It has all the foreground types required to access whatever sensor (cf. https://developer.android.com/about/versions/14/changes/fgs-types-required).

// Achieving this is by no means impossible, but requires a lot of know-how and endless hours of googling, checking documentations, and performing long-term tests.
// Read our blogpost for more details: https://claid.ethz.ch/blog/2023/06/10/everything-you-need-to-know-about-background-data-collection-on-android/ 
// Also check out the AndroidManifest for types specified for this service.


package adamma.c4dhi.claid_android.CLAIDServices;

import adamma.c4dhi.claid_android.CLAIDServices.ServiceRestartDescription;
import adamma.c4dhi.claid_android.Configuration.CLAIDPersistanceConfig;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.content.pm.ServiceInfo;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;

import android.content.Context;
import android.content.pm.ServiceInfo;

import androidx.core.app.NotificationCompat;

import adamma.c4dhi.claid.Logger.Logger;

import java.io.IOException;
import java.io.InputStream;

import adamma.c4dhi.claid_platform_impl.CLAID;


import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;


public class MaximumPermissionsPerpetualService extends CLAIDService
{
  
    private static final String CLASS_TAG = MaximumPermissionsPerpetualService.class.getName();
    public static boolean isRunning = false;
    public static final String CHANNEL_ID = "MaximumPermissionsPerpetualServiceChannel";
    public static final String ACTION_STOP_SERVICE = "adamma.c4dhi.org.claid.STOP_SERVICE";

    @Override
    public void onCreate()
    {
        super.onCreate();
        createNotificationChannel();

    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId)
    {
      



  

        Notification notification = buildNotification();

        // TODO: Add support for Android 14:
/*        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE)
        {
            startForeground(1, notification,    ServiceInfo.FOREGROUND_SERVICE_TYPE_LOCATION | ServiceInfo.FOREGROUND_SERVICE_TYPE_CAMERA | 
                                                ServiceInfo.FOREGROUND_SERVICE_TYPE_MICROPHONE | ServiceInfo.FOREGROUND_SERVICE_TYPE_HEALTH |
                                                ServiceInfo.FOREGROUND_SERVICE_TYPE_DATA_SYNC );
        }
        else  */if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R)
        {
            Logger.logInfo("Starting service >= Android R");
            startForeground(1, notification,    ServiceInfo.FOREGROUND_SERVICE_TYPE_LOCATION | ServiceInfo.FOREGROUND_SERVICE_TYPE_CAMERA | 
                                                ServiceInfo.FOREGROUND_SERVICE_TYPE_MICROPHONE );
        }
        else
        {
            startForeground(1, notification);
        }
        if(!isRunning)
        {
            isRunning = true;
            
            
            if (intent != null) 
            {                
                ServiceRestartDescription description = new ServiceRestartDescription();
                final String restartDescriptionPath = CLAID.getAppDataDirectory(this) + "/" + "claid_service_restart_description.dat";

                if(!description.deserializeFromFile(restartDescriptionPath))
                {
                    final String msg = "Failed to restart MaximumPermissionsPerpetualService. Cannot load ServiceRestartDescription from file \"" + restartDescriptionPath + "\"";
                    // This will throw a RuntimeException
                    CLAID.onUnrecoverableException(msg);
                }

                String socketPath = description.get("socketPath");
                String configFilePath = description.get("configFilePath");
                String hostId = description.get("hostId");
                String userId = description.get("userId");
                String deviceId = description.get("deviceId");

                if (socketPath == null || socketPath.isEmpty()) 
                {
                    CLAID.onUnrecoverableException("CLAID MaximumPermissionsPerpetualService failed to start: socketPath not specified in intent or is null/empty.");
                }

                if (configFilePath == null || configFilePath.isEmpty()) {
                    CLAID.onUnrecoverableException("CLAID MaximumPermissionsPerpetualService failed to start: configFilePath not specified in intent or is null/empty.");
                }

                if (hostId == null || hostId.isEmpty()) {
                    CLAID.onUnrecoverableException("CLAID MaximumPermissionsPerpetualService failed to start: hostId not specified in intent or is null/empty.");
                }

                if (userId == null || userId.isEmpty()) {
                    CLAID.onUnrecoverableException("CLAID MaximumPermissionsPerpetualService failed to start: userId not specified in intent or is null/empty.");
                }

                if (deviceId == null || deviceId.isEmpty()) {
                    CLAID.onUnrecoverableException("CLAID MaximumPermissionsPerpetualService failed to start: deviceId not specified in intent or is null/empty.");
                }

                // Do something with the extra value
                onServiceStarted(socketPath, configFilePath, hostId, userId, deviceId);
            }
            else
            {
                CLAID.onUnrecoverableException("Failed to start CLAID Service. Intent is null, so probably REDELIVER_INTENT failed.");
            }
        }

        if(ServiceManager.shouldRestartMaximumPermissionsPerpetualServiceOnTermination(this))
        {
            Logger.logInfo("Starting CLAID Service with START_REDELIVER_INTENT");
            // This will tell the OS to restart the service if it crashed or was terminated due to low resources.
            // Also will redeliver the intent, which is required because it contains the path to the CLAID config.
            return START_REDELIVER_INTENT;
        }
        else
        {
            return START_NOT_STICKY;
        }
        
    }

    void onServiceStarted(final String socketPath, final String configFilePath, final String hostId, final String userId, final String deviceId)
    {
        boolean append = true;
        try (FileWriter fileWriter = new FileWriter(CLAID.getMediaDirPath(this) + "/service_restart.txt", append)) {
            // Get the current date and time
            Date currentDate = new Date();
            SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");
            String formattedDateTime = dateFormat.format(currentDate);
    
            // Write the date and time to the file
            fileWriter.write(formattedDateTime);
            fileWriter.write(System.lineSeparator()); // Add a newline for better readability or separation
    
            // Explicitly flush the data to the file
            fileWriter.flush();
    
        } catch (IOException e) {
            System.err.println("Error writing to the file: " + e.getMessage());
        }

        Log.i(CLASS_TAG, "CLAID foreground service started");

        // Starts CLAID in the service. A registered PersistentModuleFactory will be used as ModuleFactory.
        // This requires that the user already has called CLAID.getPersistentModuleFactory(), which only works from the Application's onCreate() method.
        CLAID.onServiceStarted(this, socketPath, configFilePath, hostId, userId, deviceId);
    }

    @Override
    public void onDestroy() 
    {
        Logger.logInfo("MaximumPermissionsPerpetualService onDestroy called");
        isRunning = false;
        stopForeground(true);

        // call MyReceiver which will restart this service via a worker
        Intent broadcastIntent = new Intent(this, ServiceRestarterReceiver.class);
        sendBroadcast(broadcastIntent);

        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent)
    {
        return null;
    }

    private void createNotificationChannel()
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
        {
            NotificationChannel serviceChannel = new NotificationChannel(
                    CHANNEL_ID,
                    "Foreground Service Channel",
                    NotificationManager.IMPORTANCE_HIGH
            );
            NotificationManager manager = getSystemService(NotificationManager.class);
            manager.createNotificationChannel(serviceChannel);
        }
    }

    private Notification buildNotification()
    {
        NotificationCompat.Builder builder = new NotificationCompat.Builder(this, CHANNEL_ID)
        .setSmallIcon(adamma.c4dhi.claid.R.drawable.ic_launcher_foreground)
        .setContentTitle("CLAID Foreground Service")
        .setContentText("CLAID Foreground Service is running.")
        .setOngoing(true);

        return builder.build();
    }

    public static boolean requestRequiredPermissions(Context context, CLAIDPersistanceConfig persistanceConfig)
    {
        // Required for our Service to show an ongoing notification.
        // This notification is required to prevent that our process
        // goes into idle state, which would allow the Android OS to kill it.
        if(!CLAID.hasNotificationPermission())
        {
            if(!CLAID.requestNotificationPermission())
            {
                return false;
            }
        }
        if(!CLAID.hasMicrophonePermission())
        {
            if(!CLAID.requestMicrophonePermission())
            {
                return false;
            }
        }
        if(!CLAID.hasLocationPermission())
        {
            if(!CLAID.requestLocationPermission())
            {
                return false;
            }
        }
        Logger.logInfo("Checking battery optimization ");
        if((persistanceConfig.DISABLE_BATTERY_OPTIMIZATIONS || persistanceConfig.MONITOR_TRY_RESTART_IF_CRASHED_OR_EXITED) 
            && !CLAID.isBatteryOptimizationDisabled(context))
        {
            Logger.logInfo("Requesting exemption from battery optimization " + CLAID.isBatteryOptimizationDisabled(context));
            CLAID.requestBatteryOptimizationExemption(context); 
        }

        
        return true;
    }

   
}