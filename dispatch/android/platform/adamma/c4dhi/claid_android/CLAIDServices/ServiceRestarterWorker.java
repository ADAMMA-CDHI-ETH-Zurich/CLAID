package adamma.c4dhi.claid_android.CLAIDServices;

import adamma.c4dhi.claid_android.CLAIDServices.MaximumPermissionsPerpetualService;
import adamma.c4dhi.claid.Logger.Logger;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.core.content.ContextCompat;
import androidx.work.Worker;
import androidx.work.WorkerParameters;

public class ServiceRestarterWorker extends Worker {
    private final Context context;
    private String TAG = "CLAIDServiceRestarterWorker";

    public ServiceRestarterWorker(
            @NonNull Context context,
            @NonNull WorkerParameters params) {
        super(context, params);
        this.context = context;
        Logger.logInfo("ServiceRestarterWorker created.");
    }

    @NonNull
    @Override
    public Result doWork() {
        Logger.logInfo("doWork called for: " + this.getId());
        Logger.logInfo("Service Running: " + MaximumPermissionsPerpetualService.isRunning);
        if (!MaximumPermissionsPerpetualService.isRunning) 
        {
            Logger.logInfo("Starting service from doWork");
            ServiceManager.restartMaximumPermissionsPerpetualServiceByIntent(context);
        }
        else
        {
            Logger.logInfo("ServiceRestarterWorker service is running, going back to sleep zzzzZZZZzzz");
        }
        return Result.success();
    }

    @Override
    public void onStopped() {
        Logger.logInfo("onStopped called for: " + this.getId());
        super.onStopped();
    }
}