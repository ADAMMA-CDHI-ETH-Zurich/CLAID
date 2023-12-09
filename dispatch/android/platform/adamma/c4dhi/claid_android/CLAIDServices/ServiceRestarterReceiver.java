package adamma.c4dhi.claid_android.CLAIDServices;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid_android.CLAIDServices.ServiceRestarterWorker;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.work.OneTimeWorkRequest;
import androidx.work.WorkManager;

public class ServiceRestarterReceiver extends BroadcastReceiver 
{
    private String TAG = "CLAIDServiceRestarterReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        Logger.logInfo("ServiceRestarterReciver onReceive called.");

        // We are starting MyService via a worker and not directly because since Android 7
        // (but officially since Lollipop!), any process called by a BroadcastReceiver
        // (only manifest-declared receiver) is run at low priority and hence eventually
        // killed by Android.
        if(ServiceManager.shouldRestartMaximumPermissionsPerpetualServiceOnTermination(context))
        {
            Logger.logInfo("ServiceRestarterReceiver: Restarting service as configured.");
            WorkManager workManager = WorkManager.getInstance(context);
            OneTimeWorkRequest startServiceRequest = new OneTimeWorkRequest.Builder(ServiceRestarterWorker.class)
                    .build();
            workManager.enqueue(startServiceRequest);
        }
        else
        {
            Logger.logInfo("ServiceRestarterReceiver: Not restarting service as configured.");
        }
        
    }
}