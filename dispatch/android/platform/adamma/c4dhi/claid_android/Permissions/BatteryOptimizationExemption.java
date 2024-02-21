package adamma.c4dhi.claid_android.Permissions;

import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.PowerManager;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid_android.Permissions.BatteryOptimizationExemptionActivity;
import adamma.c4dhi.claid_android.Permissions.Permission;

public class BatteryOptimizationExemption extends Permission
{
    @Override
    public boolean isGranted() 
    {
        Context context = super.getContext();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
        {
            Logger.logInfo("Checking battery optimization 2");

            PowerManager powerManager = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
            Logger.logInfo("Checking battery optimization 3 " + context.getPackageName());

            return powerManager.isIgnoringBatteryOptimizations(context.getPackageName());
        }
        // For versions prior to Android M, battery optimization is not enabled.
        return true;

    }

    @Override
    public void blockingRequest() 
    {
        if (isGranted()) {
            Logger.logInfo("We have battery optimization exemtpion.");
            return;
        }
        Logger.logInfo("Checking battery optimization 4");

        Intent intent = new Intent(getContext(), BatteryOptimizationExemptionActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        Logger.logInfo("request microphone permission6");

        getContext().startActivity(intent);
        Logger.logInfo("Checking battery optimization 5");

    }
}
