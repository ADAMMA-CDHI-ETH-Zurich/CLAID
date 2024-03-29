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

package adamma.c4dhi.claid_android.Receivers;
import adamma.c4dhi.claid_android.CLAIDServices.ServiceManager;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Debug;
import android.util.Log;

import java.io.FileOutputStream;
import java.security.Provider.Service;
import java.text.SimpleDateFormat;
import java.util.Date;

import adamma.c4dhi.claid.Logger.Logger;

public class OnBootReceiver extends BroadcastReceiver
{
    @Override
    public void onReceive(Context context, Intent intent)
    {
        
        try {
            
            Logger.logInfo("onBootReceiver triggered");
            String action = intent.getAction();

            if(action != null)
            {
                if (action.equals(Intent.ACTION_BOOT_COMPLETED) )
                {
                    Logger.logInfo("onBootReceiver is ACTION_BOOT_COMPLETED");
                    if(ServiceManager.shouldStartMaximumPermissionsPerpetualServiceOnBoot(context))
                    {
                        Logger.logInfo("onBootReceiver shouldStartMaximumPermissionsPerpetualServiceOnBoot is true, trying to start Service.");
                        ServiceManager.executeConfiguredStartupMethodForMaximumPermissionsPerpetualService(context);
                    }
                }
            }
        } 
        catch (Exception e)
        {
            Logger.logFatal(e.getMessage());
        }
        Logger.logInfo("CLAID onBoot !");

    }
}