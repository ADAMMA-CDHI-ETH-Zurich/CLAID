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
import adamma.c4dhi.claid.Logger.Logger;


public class MicrophonePermission extends Permission {
    private static final int MICROPHONE_REQUEST_CODE = 500;
    private static final String userDialogTitle = "You need to allow microphone permission";
    private static final String userDialogBody = "In the following screen you need to allow " +
            "microphone permissions to use this app. If you can't see the option you need " +
            "to open\n" + "Settings->Apps->CLAIDDemo->Permissions.";
    private static final String[] RECORDING_PERMISSION = {
            Manifest.permission.RECORD_AUDIO
    };


    @Override
    public boolean isGranted() {
        return (super.getContext().checkSelfPermission(RECORDING_PERMISSION[0])
                == PackageManager.PERMISSION_GRANTED);
    }

    @Override
    public void blockingRequest() {
        Logger.logInfo("request microphone permission3");

        if (!isGranted()) {
            Logger.logInfo("Looping1");

            super.startIntentWithExtras(RECORDING_PERMISSION, MICROPHONE_REQUEST_CODE, userDialogTitle, userDialogBody);
            Logger.logInfo("Looping2");

            try{
                Thread.sleep(200);
            }
            catch(InterruptedException e)
            {

            }
        }
        while (!isGranted()) {}
        Logger.logInfo("granted");

    }
}
