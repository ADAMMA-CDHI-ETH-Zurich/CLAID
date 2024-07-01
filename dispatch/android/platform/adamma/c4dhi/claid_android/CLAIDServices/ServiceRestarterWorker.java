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

        Logger.logInfo("ServiceRestarterWorker woke up.");
        if(!ServiceManager.shouldRestartMaximumPermissionsPerpetualServiceOnTermination(context))
        {  
            Logger.logInfo("ServiceRestarterWorker: Config says we shall not restart CLAID service if crashed. Going back to sleep.");
            return Result.success();
        }
        Logger.logInfo("doWork called for: " + this.getId());
        Logger.logInfo("Service Running: " + MaximumPermissionsPerpetualService.isRunning);
        if (!MaximumPermissionsPerpetualService.isRunning) 
        {
            Logger.logInfo("CLAID service is not active anymore. Maybe it crashed. Restarting it.");
            ServiceManager.restartMaximumPermissionsPerpetualServiceByIntent(context);
        }
        else
        {
            Logger.logInfo("ServiceRestarterWorker CLAID services is running, going back to sleep zzzzZZZZzzz");
        }
        return Result.success();
    }

    @Override
    public void onStopped() {
        Logger.logInfo("onStopped called for: " + this.getId());
        super.onStopped();
    }
}