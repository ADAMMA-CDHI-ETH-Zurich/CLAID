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
import adamma.c4dhi.claid.Logger.Logger;


public class BluetoothPermission extends Permission {
     private static final int BLUETOOTH_REQUEST_CODE = 301;
     private static final String userDialogTitle = "You need to allow bluetooth permissions";
     private static final String userDialogBody = "In the following screen you need to allow " +
             "bluetooths permissions to use this app. If you can't see the option you need " +
             "to open\nSettings->Apps->Select this App->Permissions.";
     private static final String[] BLUETOOTH_PERMISSION = {
             Manifest.permission.BLUETOOTH_CONNECT,
             Manifest.permission.BLUETOOTH_SCAN
     };


    @Override
    public boolean isGranted()
    {    
        for(String permission : BLUETOOTH_PERMISSION)
        {
            if(super.getContext().checkSelfPermission(permission) != PackageManager.PERMISSION_GRANTED)
            {
                return false;
            }
        }
        return true;
    }

    @Override
     public void blockingRequest() {
        if (!isGranted()) 
        {
            Thread thread = new Thread(() -> {
                try {
                    startIntentWithExtras(BLUETOOTH_PERMISSION, BLUETOOTH_REQUEST_CODE, userDialogTitle, userDialogBody);
                } catch (Exception e) {
                    // Handle the exception here
                    e.printStackTrace(); // This will print the exception stack trace, replace it with appropriate handling
                }
            });
            
            thread.start();
            try {
                Logger.logInfo("Joining 1");
                thread.join();
            } catch (InterruptedException e) {
                // Handle interruption exception if needed
                Logger.logInfo("Joining 2");

                e.printStackTrace(); // Replace it with appropriate handling
            }
            
            while (!isGranted()) {   
                
                Logger.logInfo("Joining 3");

                try{
                    Thread.sleep(20);
                }
                catch(InterruptedException e)
                {
                    e.printStackTrace();
                }
    
        }


        }
    }

 }
