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
import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;


public class NotificationPermission extends Permission {
     private static final int NOTIFICATION_REQUEST_CODE = 300;
     private static final String userDialogTitle = "You need to allow notification permission";
     private static final String userDialogBody = "In the following screen you need to allow " +
             "notification permissions to use this app. If you can't see the option you need " +
             "to open\nSettings->Apps->YourCLAIDApp->Permissions.";
     private static final String[] NOTIFICATION_PERMISSION = {
             Manifest.permission.POST_NOTIFICATIONS
     };


    @Override
    public boolean isGranted() {
        if (Build.VERSION.SDK_INT >= 33) {
            return super.getContext().checkSelfPermission(
                    Manifest.permission.POST_NOTIFICATIONS) == PackageManager.PERMISSION_GRANTED;
        }
        return true;
    }

    @Override
     public void blockingRequest() {
        if (!isGranted() && Build.VERSION.SDK_INT >= 33) {
            startIntentWithExtras(NOTIFICATION_PERMISSION, NOTIFICATION_REQUEST_CODE, userDialogTitle, userDialogBody);
            while (!isGranted()) {}
        }
    }

 }
