package adamma.c4dhi.claid_android.UserFeedback;
import android.annotation.SuppressLint;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.os.Build;
import androidx.core.app.NotificationCompat;
import adamma.c4dhi.claid_android.Permissions.NotificationPermission;

import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.ChannelData;
import adamma.c4dhi.claid.Module.Module;



import adamma.c4dhi.claid_platform_impl.CLAID;

import adamma.c4dhi.claid.Module.ModuleAnnotator;
import adamma.c4dhi.claid.Module.PropertyHelper.PropertyHelper;

import java.util.Map;


public class NotificationModule extends Module
{
    Context context;
    private Channel<String> notificationDataChannel;
    private NotificationManager notificationManager;
    private final String CHANNEL_NAME = "NotificationChannel";
    private int notificationID = 2;
    private String notificationChannelID = "2";

    public static void annotateModule(ModuleAnnotator annotator)
    {
        annotator.setModuleCategory("UserFeedback");
        annotator.setModuleDescription("The NotifciationModule displays notifications to the user based on a text received on it's input channel.");

 
        annotator.describeSubscribeChannel("NotificationText", String.class, "Text to show to the user in a notification.");
    }


    public void initialize(Map<String, String> initialize)
    {
        moduleInfo("NotificationModule Initialize");
        return;/*
        //new NotificationPermission().blockingRequest();
        this.notificationDataChannel = this.subscribe("NotificationText", String.class, (data) -> onNotificationRequest(data));
        context = (Context) CLAID.getContext();
        notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);*/
    }

    public void onNotificationRequest(ChannelData<String> data)
    {
        String text = data.getData();
        displayNotification("CLAID", text);
    }

    public void displayNotification(String title, String body)
    {

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
        {
            NotificationChannel channel = new NotificationChannel(notificationChannelID, "notificationChannel", NotificationManager.IMPORTANCE_DEFAULT);
            notificationManager.createNotificationChannel(channel);
        }

        @SuppressLint("DiscouragedApi") NotificationCompat.Builder builder = new NotificationCompat.Builder(context, notificationChannelID)
                .setSmallIcon(context.getResources().getIdentifier("ic_launcher_background", "drawable", context.getPackageName()))
                .setContentTitle(title)
                .setContentText(body)
                .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                .setAutoCancel(true);

        notificationManager.notify(notificationID++, builder.build());

    }

}