/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

package adamma.c4dhi.claid_android.Permissions;

import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.PowerManager;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid_android.Permissions.BatteryOptimizationExemptionActivity;
import adamma.c4dhi.claid_android.Permissions.Permission;
import adamma.c4dhi.claid_platform_impl.CLAID;
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
import java.util.List;


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
            Logger.logInfo("We have battery optimization exemption.");
            return;
        }
        Logger.logInfo("Checking battery optimization 4");

        Intent intent = new Intent(getContext(), BatteryOptimizationExemptionActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        Logger.logInfo("request microphone permission6");

        getContext().startActivity(intent);
        Logger.logInfo("Checking battery optimization 5");
        try{
            Thread.sleep(2000);
        }
        catch(InterruptedException e)
        {
            e.printStackTrace();
        }
        Logger.logInfo("Checking battery optimization testx " + isGranted() + " " + isAppOnForeground());

        while(!isGranted() || !isAppOnForeground())
        {
            Logger.logInfo("Checking battery optimization test " + isGranted() + " " + isAppOnForeground());

        }
    }

    public boolean isAppOnForeground() {
        ActivityManager activityManager = (ActivityManager) this.getContext().getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> appProcesses = activityManager.getRunningAppProcesses();
        if (appProcesses == null) {
            return false;
        }
        final String packageName = this.getContext().getPackageName();
        for (ActivityManager.RunningAppProcessInfo appProcess : appProcesses) {
            if (appProcess.importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_FOREGROUND && appProcess.processName.equals(packageName)) {
                return true;
            }
        }
        return false;
    }

}
