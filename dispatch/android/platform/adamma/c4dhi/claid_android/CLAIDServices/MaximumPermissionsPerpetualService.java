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


import android.content.pm.ServiceInfo;


import adamma.c4dhi.claid.Logger.Logger;

import java.io.IOException;
import java.io.InputStream;

import adamma.c4dhi.claid_platform_impl.CLAID;

public class MaximumPermissionsPerpetualService extends CLAIDService
{
  
    private static final String CLASS_TAG = MaximumPermissionsPerpetualService.class.getName();
    private boolean isRunning = false;
    public static final String CHANNEL_ID = "MaximumPermissionsPerpetualServiceChannel";


    @Override
    public void onCreate()
    {
        super.onCreate();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId)
    {
        createNotificationChannel();
        Intent notificationIntent = new Intent(this, MaximumPermissionsPerpetualService.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(this,
                0, notificationIntent, PendingIntent.FLAG_IMMUTABLE);



        Notification.Builder builder;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            builder = new Notification.Builder(this, CHANNEL_ID);
        } else {
            builder = new Notification.Builder(this);
        }

        builder.setContentTitle("CLAID Foreground Service")
            .setContentText("CLAID Foreground Service is running.")
            .setOngoing(true);

        Notification notification = builder.build();

        // TODO: Add support for Android 14:
/*        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE)
        {
            startForeground(1, notification,    ServiceInfo.FOREGROUND_SERVICE_TYPE_LOCATION | ServiceInfo.FOREGROUND_SERVICE_TYPE_CAMERA | 
                                                ServiceInfo.FOREGROUND_SERVICE_TYPE_MICROPHONE | ServiceInfo.FOREGROUND_SERVICE_TYPE_HEALTH |
                                                ServiceInfo.FOREGROUND_SERVICE_TYPE_DATA_SYNC );
        }
        else  */if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R)
        {
            startForeground(1, notification,    ServiceInfo.FOREGROUND_SERVICE_TYPE_LOCATION | ServiceInfo.FOREGROUND_SERVICE_TYPE_CAMERA | 
                                                ServiceInfo.FOREGROUND_SERVICE_TYPE_MICROPHONE );
        }
        else
        {
            startForeground(1, notification);
        }
        if(!this.isRunning)
        {
            this.isRunning = true;
            
            
            if (intent != null) 
            {                
                String socketPath = intent.getStringExtra("socketPath");
                String configFilePath = intent.getStringExtra("configFilePath");
                String hostId = intent.getStringExtra("hostId");
                String userId = intent.getStringExtra("userId");
                String deviceId = intent.getStringExtra("deviceId");

                if (socketPath == null || socketPath.isEmpty()) {
                    Logger.logError("CLAID MaximumPermissionsPerpetualService failed to start: socketPath not specified in intent or is null/empty.");
                    System.exit(0);
                }

                if (configFilePath == null || configFilePath.isEmpty()) {
                    Logger.logError("CLAID MaximumPermissionsPerpetualService failed to start: configFilePath not specified in intent or is null/empty.");
                    System.exit(0);
                }

                if (hostId == null || hostId.isEmpty()) {
                    Logger.logError("CLAID MaximumPermissionsPerpetualService failed to start: hostId not specified in intent or is null/empty.");
                    System.exit(0);
                }

                if (userId == null || userId.isEmpty()) {
                    Logger.logError("CLAID MaximumPermissionsPerpetualService failed to start: userId not specified in intent or is null/empty.");
                    System.exit(0);
                }

                if (deviceId == null || deviceId.isEmpty()) {
                    Logger.logError("CLAID MaximumPermissionsPerpetualService failed to start: deviceId not specified in intent or is null/empty.");
                    System.exit(0);
                }

                // Do something with the extra value
                onServiceStarted(socketPath, configFilePath, hostId, userId, deviceId);
            }
            else
            {
                Logger.logError("Failed to start CLAID Service. Intent is null, so probably REDELIVER_INTENT failed.");
                System.exit(0);
            }
        }

        // This will tell the OS to restart the service if it crashed or was terminated due to low resources.
        // Also will redeliver the intent, which is required because it contains the path to the CLAID config.
        return START_REDELIVER_INTENT;
    }

    void onServiceStarted(final String socketPath, final String configFilePath, final String hostId, final String userId, final String deviceId)
    {
        Log.i(CLASS_TAG, "CLAID foreground service started");

        // Starts CLAID in the service. A registered PersistentModuleFactory will be used as ModuleFactory.
        // This requires that the user already has called CLAID.getPersistentModuleFactory(), which only works from the Application's onCreate() method.
        CLAID.onServiceStarted(this, socketPath, configFilePath, hostId, userId, deviceId);
    }

    @Override
    public void onDestroy()
    {
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
                    NotificationManager.IMPORTANCE_DEFAULT
            );
            NotificationManager manager = getSystemService(NotificationManager.class);
            manager.createNotificationChannel(serviceChannel);
        }
    }

    public static boolean requestRequiredPermissions()
    {
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
        if(!CLAID.hasStoragePermission())
        {
            if(!CLAID.requestStoragePermission())
            {
                return false;
            }
        }
        return true;
    }
}