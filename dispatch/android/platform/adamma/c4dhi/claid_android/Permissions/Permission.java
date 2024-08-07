/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Francesco Feher
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

import static adamma.c4dhi.claid_android.Permissions.PermissionActivity.EXTRA_DIALOG_BODY;
import static adamma.c4dhi.claid_android.Permissions.PermissionActivity.EXTRA_DIALOG_TITLE;
import static adamma.c4dhi.claid_android.Permissions.PermissionActivity.EXTRA_PERMISSIONS;
import static adamma.c4dhi.claid_android.Permissions.PermissionActivity.EXTRA_REQUEST_CODE;
import static adamma.c4dhi.claid_android.Permissions.PermissionActivity.EXTRA_REQUIRES_PERMISSION_PAGE;
import static adamma.c4dhi.claid_android.Permissions.PermissionActivity.EXTRA_ALWAYS_SHOW_INFO_DIALOG;;

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import java.util.List;

import androidx.core.content.IntentCompat;

import adamma.c4dhi.claid_platform_impl.CLAID;
import adamma.c4dhi.claid.Logger.Logger;
public abstract class Permission
{
    public abstract void blockingRequest();

    public abstract boolean isGranted();

    public void startIntentWithExtras(String[] permissions, int requestCode, String dialogTitle, String dialogBody)
    {
        startIntentWithExtras(permissions, requestCode, dialogTitle, dialogBody, false, false);
    }

    public void startIntentWithExtras(String[] permissions, int requestCode, String dialogTitle, String dialogBody, boolean permissionsPage)
    {
        startIntentWithExtras(permissions, requestCode, dialogTitle, dialogBody, permissionsPage, false);
    }

    public void startIntentWithExtras(String[] permissions, int requestCode, String dialogTitle, String dialogBody, boolean permissionsPage, boolean alwaysShowInfoDialog)
    {
        Logger.logInfo("request microphone permission4");

        while (!isAppOnForeground()){}
        Logger.logInfo("request microphone permission5");

        Intent intent = new Intent(getContext(), PermissionActivity.class);
        intent.putExtra(EXTRA_PERMISSIONS, permissions);
        intent.putExtra(EXTRA_REQUEST_CODE, requestCode);
        intent.putExtra(EXTRA_DIALOG_TITLE, dialogTitle);
        intent.putExtra(EXTRA_DIALOG_BODY, dialogBody);
        intent.putExtra(EXTRA_REQUIRES_PERMISSION_PAGE, permissionsPage);
        intent.putExtra(EXTRA_ALWAYS_SHOW_INFO_DIALOG, alwaysShowInfoDialog);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        Logger.logInfo("request microphone permission6");

        getContext().startActivity(intent);
        Logger.logInfo("request microphone permission7");

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

    private static Context context = null;

    public static void setContext(Context context)
    {
        Permission.context = context;
    }

    public Context getContext() 
    {
        if(Permission.context != null)
        {
            return Permission.context;

        }
        return (Context) CLAID.getContext();
    }

    protected void sleepABit()
    {
        try{
            Thread.sleep(20);
        }
        catch(InterruptedException e)
        {
            
        }
    }


}
