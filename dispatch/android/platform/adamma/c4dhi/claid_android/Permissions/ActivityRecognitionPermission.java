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
import android.content.pm.PackageManager;
import android.os.Build;


public class ActivityRecognitionPermission extends Permission {
     private static final int ACTIVITY_PERMISSION_REQUEST_CODE = 333;
     private static final String userDialogTitle = "You need to allow activity recognition permission.";
     private static final String userDialogBody = "Please allow permissions for physical activity recognition. "+
        "When clicking ok, you will be redirected to the App settings page. Please choose permissions and grant the " +
        "physical activity recognition permission.";
     private static final String[] activityRecognitionPermissions = {
             Manifest.permission.ACTIVITY_RECOGNITION
     };


    @Override
    public boolean isGranted() {
        if (Build.VERSION.SDK_INT >= 33) {
            return super.getContext().checkSelfPermission(
                    Manifest.permission.ACTIVITY_RECOGNITION) == PackageManager.PERMISSION_GRANTED;
        }
        return true;
    }

    @Override
     public void blockingRequest() {
        if (!isGranted() && Build.VERSION.SDK_INT >= 33) {
            startIntentWithExtras(activityRecognitionPermissions, ACTIVITY_PERMISSION_REQUEST_CODE, userDialogTitle, userDialogBody);
            while (!isGranted()) {}
        }
    }

 }
