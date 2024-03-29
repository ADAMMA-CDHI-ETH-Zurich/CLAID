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