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

package adamma.c4dhi.claid_android.Dialog;

import static adamma.c4dhi.claid_android.Dialog.DialogActivity.EXTRA_DIALOG_BODY;
import static adamma.c4dhi.claid_android.Dialog.DialogActivity.EXTRA_DIALOG_TITLE;


import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

import adamma.c4dhi.claid_platform_impl.CLAID;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid_android.Dialog.DialogActivity;
public class Dialog
{


    public void showDialogBlocking(String dialogTitle, String dialogBody)
    {
        // Wait until dialog is hidden.
        // That means another component is currently showing a dialog and we need to wait until it is done.
        if(DialogActivity.isShown == null)
        {

            DialogActivity.isShown = new AtomicBoolean();
            DialogActivity.isShown.set(false);
        }
        while(DialogActivity.isShown.get())
        {

        }

        while (!isAppOnForeground()){}


        Intent intent = new Intent(getContext(), DialogActivity.class);

        intent.putExtra(EXTRA_DIALOG_TITLE, dialogTitle);
        intent.putExtra(EXTRA_DIALOG_BODY, dialogBody);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        DialogActivity.isShown.set(true);

        getContext().startActivity(intent);

        // Wait until dialog is hidden.
        while(DialogActivity.isShown.get())
        {

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

    private static Context context = null;



    public Context getContext() 
    {
        return CLAID.getContext();
    }

}
