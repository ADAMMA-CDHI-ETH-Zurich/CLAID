package adamma.c4dhi.claid_android.Permissions;
import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.os.Bundle;
import android.os.PowerManager;
import android.provider.Settings;
import android.view.WindowManager;
import android.app.Activity;
import android.app.AlertDialog;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid_platform_impl.CLAID;

import java.util.Arrays;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.atomic.AtomicBoolean;

// "Invisible" activity used to ask permissions
public class BatteryOptimizationExemptionActivity extends Activity {
   

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        this.setTheme();
       
        this.requestBatteryOptimizationExemption();
    }

    private void requestBatteryOptimizationExemption()
    {
        AtomicBoolean buttonClicked = new AtomicBoolean();
        buttonClicked.set(false);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) 
        {
            Context context = this;
            if(!CLAID.isWearOS(context))
            {
                runOnUiThread(() ->
                    new AlertDialog.Builder(context)
                    .setMessage("On the next page, please disable battery optimizations. This is required to make CLAID run in the background as long as possible.")
                    .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            buttonClicked.set(true);
                            showBatteryOptimizationIntent(context);
                        }
                    })
                    .show());
               
                    while(!buttonClicked.get())
                    {
                        try {
                            Thread.sleep(20);
                        } catch (InterruptedException e) {
                            throw new RuntimeException(e);
                        }
                    }
        
                    while(!isAppOnForeground())
                    {
                        try {
                            Thread.sleep(20);
                        } catch (InterruptedException e) {
                            throw new RuntimeException(e);
                        }
                    }
        
                    BatteryOptimizationExemption exemption = new BatteryOptimizationExemption();
                    if(!exemption.isGranted())
                    {
                        requestBatteryOptimizationExemption();
                    }
            }
            else
            {
                String batteryOptmizationMessage = 
                "For CLAID services to run as long as possible and being able to restart silently from the background, it is necessary\n" +
                "to disable battery optimizations for this App. On WearOS, however, it is currently not possible " + 
                "to request the user to disable battery optimizations (in contrast to regular Android).\n" +
                "On WearOS, battery optimizations can currently ONLY be disabled using ADB (Android Debug Bridge).\n" + 
                "Connect ADB to your device and enter the following command: \"adb shell dumpsys deviceidle whitelist +" + context.getPackageName() + "\".\n" +
                "Restart the App afterwards, and battery optimizations should be disabled. You can verify this using CLAID.isBatteryOptimizationDisabled().";

              
                runOnUiThread(() ->
                    new AlertDialog.Builder(context)
                    .setMessage(batteryOptmizationMessage)
                    .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            buttonClicked.set(true);
                            showBatteryOptimizationIntent(context);
                        }
                    })
                    .show());
               
            
            }

            while(!buttonClicked.get())
            {
                try {
                    Thread.sleep(20);
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
            }

            while(!isAppOnForeground())
            {
                try {
                    Thread.sleep(20);
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
            }

            PackageManager packageManager = this.getPackageManager();
            Intent intent = packageManager.getLaunchIntentForPackage(this
                    .getPackageName());
            ComponentName componentName = intent.getComponent();
            Intent mainIntent = Intent.makeRestartActivityTask(componentName);
            this.startActivity(mainIntent);
            Runtime.getRuntime().exit(0);
        }
        else
        {
            finish();
        }
    }

    private static void showBatteryOptimizationIntent(Context context)
    {
        Intent intent = new Intent();
        String packageName = context.getPackageName();
        PowerManager powerManager = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
    

        if (!powerManager.isIgnoringBatteryOptimizations(packageName)) {
            intent.setAction(Settings.ACTION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS);
            intent.setData(android.net.Uri.parse("package:" + packageName));
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent);
        }
    }

    private void displayAlertDialog(String dialogTitle, String dialogBody)
    {
        Logger.logInfo("request microphone permission11");

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(dialogTitle);
        builder.setMessage(dialogBody);
        builder.setPositiveButton("OK", (dialog, id) -> {
            PackageManager packageManager = this.getPackageManager();
            Intent intent = packageManager.getLaunchIntentForPackage(this
                    .getPackageName());
            ComponentName componentName = intent.getComponent();
            Intent mainIntent = Intent.makeRestartActivityTask(componentName);
            this.startActivity(mainIntent);
            Runtime.getRuntime().exit(0);
        });
        builder.setCancelable(false);
        builder.setIcon(android.R.drawable.ic_dialog_alert);
        builder.show();
    }

    private void setTheme()
    {

        getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            getWindow().setDecorFitsSystemWindows(false);
        }
        getWindow().setWindowAnimations(0);
        getWindow().setDimAmount(0.0f);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
                WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);
    }

    public boolean isAppOnForeground() {
        ActivityManager activityManager = (ActivityManager) getApplicationContext().getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> appProcesses = activityManager.getRunningAppProcesses();
        if (appProcesses == null) {
            return false;
        }
        final String packageName = getApplicationContext().getPackageName();
        for (ActivityManager.RunningAppProcessInfo appProcess : appProcesses) {
            if (appProcess.importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_FOREGROUND && appProcess.processName.equals(packageName)) {
                return true;
            }
        }
        return false;
    }


}