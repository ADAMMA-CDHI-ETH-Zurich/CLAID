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
import android.Manifest;
import android.annotation.SuppressLint;
import android.content.pm.PackageManager;
import android.os.Build;
import adamma.c4dhi.claid.Logger.Logger;

import adamma.c4dhi.claid_platform_impl.CLAID;

public class BodySensorsPermission extends Permission {
    private static final int BODY_SENSORS_REQUEST_CODE = 201;
    private static final String userDialogTitleForeground = "Permissions for body sensors (foregorund)";
    private static final String userDialogBodyForeground = "You need to allow permissions for body sensors to use this App.";
    private static final String userDialogTitleBackground = "Permissions for body sensors (background)";
    private static final String userDialogBodyBackground = "Please enable the permissions for the body sensors at all times. " +
                                                           "When clicking ok, you will be redirected to the app settings or permissions page. "+ 
                                                           "There, please select \"sensors\" and allow access at all times.";

    
    

    private final String[] stringPermissionsForeground = new String[]{
            Manifest.permission.BODY_SENSORS,
    };
    @SuppressLint("InlinedApi")
    private final String[] stringPermissionsBackground = new String[]{
            Manifest.permission.BODY_SENSORS_BACKGROUND
    };

    @Override
    public boolean isGranted() {
   
        return (isBodySensorsForegroundGranted() && isBodySensorsBackgroundGranted());
    }

    public boolean isBodySensorsForegroundGranted() {
        return super.getContext().checkSelfPermission(Manifest.permission.BODY_SENSORS) ==
                PackageManager.PERMISSION_GRANTED;
    }

    public boolean isBodySensorsBackgroundGranted() {
        return super.getContext().checkSelfPermission(Manifest.permission.BODY_SENSORS_BACKGROUND) ==
                PackageManager.PERMISSION_GRANTED;
    }

    @Override
    public void blockingRequest() {
            if (isGranted()) {
                Logger.logInfo("We have body sensor permissions");
                return;
            }
         
            Logger.logInfo("location 1");
            if (!isBodySensorsForegroundGranted()) {
                Logger.logInfo("location 2");

                super.startIntentWithExtras(
                    stringPermissionsForeground, 
                    BODY_SENSORS_REQUEST_CODE, 
                    userDialogTitleForeground, 
                    userDialogBodyForeground
                );
            }

            while(!isBodySensorsForegroundGranted())
            {
                Logger.logInfo("Sleeping waiting for foreground");
                sleepABit();
            }
            Logger.logInfo("waiting for background");

            // Use while here, because the background permission for
            // body sensors can only be set from the permissions page.
            // Hence, we have to send the user to the permissions page and wait for him
            // to grant the permission. Note the extra parameter in startIntentWithExtras.
            if (!isBodySensorsBackgroundGranted()) {
                super.startIntentWithExtras(
                    stringPermissionsBackground, 
                    BODY_SENSORS_REQUEST_CODE, 
                    userDialogTitleBackground, 
                    userDialogBodyBackground,
                    true,
                    true
                );
            }

            while (!isGranted() || !isAppOnForeground()){ 
               
                sleepABit();
            }
    }


}

