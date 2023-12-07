// This class provides CLAID's effort to provide a Service that is able 
// to perform everything in the background (at least everything that Android still allows),
// and is as  persistent as possible on the Android OS, without having root privileges (i.e., without being a system service).
// It runs as long as possible, auto starts on boot, restarts after a crash and ideally even after being terminated by the OS due to resource restrictions.
// Optionally, it can also prevent the device from going to doze or deep sleep by using WakeLocks.
// It has all the foreground types required to access whatever sensor (cf. https://developer.android.com/about/versions/14/changes/fgs-types-required).

// Achieving this is by no means impossible, but requires a lot of know-how and endless hours of googling, checking documentations, and performing long-term tests.
// Read our blogpost for more details: https://claid.ethz.ch/blog/2023/06/10/everything-you-need-to-know-about-background-data-collection-on-android/ 
// Also check out the AndroidManifest for types specified for this service.


package adamma.c4dhi.claid_android;

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

import androidx.annotation.Nullable;
import androidx.core.app.NotificationCompat;





import java.io.IOException;
import java.io.InputStream;

import JavaCLAID.CLAID;

public class MaximumPermissionsPerpetualService extends Service
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
        Notification notification = new NotificationCompat.Builder(this, CHANNEL_ID)
                .setContentTitle("Foreground Service")
                .setContentText("CLAID Foreground Service")
                .setOngoing(true)
                .setPriority(NotificationCompat.PRIORITY_HIGH)
                .build();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R)
        {
            startForeground(1, notification,    FOREGROUND_SERVICE_TYPE_LOCATION | FOREGROUND_SERVICE_TYPE_CAMERA | 
                                                FOREGROUND_SERVICE_TYPE_MICROPHONE | FOREGROUND_SERVICE_TYPE_HEALTH |
                                                FOREGROUND_SERVICE_TYPE_DATA_SYNC | FOREGROUND_SERVICE_TYPE_FILE_MANAGEMENT);
        }
        else
        {
            startForeground(1, notification);
        }
        if(!this.isRunning)
        {
            this.isRunning = true;
            onServiceStarted();
        }

        // This will tell the OS to restart the service if it crashed.
        // Also will redeliver the intent, which is required because it contains the path to the CLAID config.
        return START_REDELIVER_INTENT;
    }

    void onServiceStarted(final String configPath)
    {
        Log.i(CLASS_TAG, "CLAID foreground service started");
        String assetsXMLConfig = loadFileFromAssets("CLAID.xml");
        CLAID.
        //CLAID.enableLoggingToFile("/storage/emulated/0/Android/media/adamma.c4dhi.claid.wearos/" + "/CLAIDLog.txt");
    }

    @Override
    public void onDestroy()
    {
        super.onDestroy();
    }

    @Nullable
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
}