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

package adamma.c4dhi.claid_android.Configuration;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
// A configuration specifying the power ("mightiness")
// CLAID has to perform "above standard" functions (i.e., 
// functions that are typically not available for regular Android apps, such as disabling or enabling Wifi from the background without user intervention).

// In most cases, CLAIDSpecialPermissionsConfig.regularConfig() is sufficient and the CLAID App will have as much power as any regular Android app.
// For some cases, you might require more Power, e.g., to manage the external storage, start activities from the background, disable or enable wifi without user intervention.


public class CLAIDSpecialPermissionsConfig 
{
    // Allows the CLAID app to manage all files on the internal storage or external sd card.
    public boolean MANAGE_ALL_STORAGE = false;

    // Allows to restart activities from the background and makes Android more reluctant to kill the CLAID service.
    public boolean DISABLE_BATTERY_OPTIMIZATIONS = false;

    // Device Owner Apps can do a lot more than regular Apps on modern Android OS versions (or, can do what regular Apps were allowed to do in old Android versions anyways).
    // Functions such as disabling or enabling wifi or bluetooth from the background, without the user having to intervene,
    // are available for device owner apps. Device owner Apps can take major control over the OS.
    public boolean BE_DEVICE_OWNER = false;

    public static CLAIDSpecialPermissionsConfig almightyCLAID()
    {
        CLAIDSpecialPermissionsConfig config = new CLAIDSpecialPermissionsConfig();
        config.MANAGE_ALL_STORAGE = true;
        config.DISABLE_BATTERY_OPTIMIZATIONS = true;
        config.BE_DEVICE_OWNER = true;

        return config;
    }
    
    public static CLAIDSpecialPermissionsConfig regularConfig()
    {
        CLAIDSpecialPermissionsConfig config = new CLAIDSpecialPermissionsConfig();
        config.MANAGE_ALL_STORAGE = false;
        config.DISABLE_BATTERY_OPTIMIZATIONS = false;
        config.BE_DEVICE_OWNER = false;

        return config;
    }

    public static CLAIDSpecialPermissionsConfig allStorageAccessConfig()
    {
        CLAIDSpecialPermissionsConfig config = new CLAIDSpecialPermissionsConfig();

        config.MANAGE_ALL_STORAGE = true;
        config.DISABLE_BATTERY_OPTIMIZATIONS = false;
        config.BE_DEVICE_OWNER = false;

        return config;
    }

    public static boolean writeToFile(final String path, CLAIDSpecialPermissionsConfig config)
    {
        try {
            FileOutputStream fileOut = new FileOutputStream(path);
            ObjectOutputStream out = new ObjectOutputStream(fileOut);
            out.writeObject(config);
            out.close();
            fileOut.close();
        } catch (IOException i) {
            i.printStackTrace();
            return false;
        }
        return true;
    }

    public CLAIDSpecialPermissionsConfig readFromFile(final String path)
    {
        CLAIDSpecialPermissionsConfig deserialized;
        try {
            FileInputStream fileIn = new FileInputStream(path);
            ObjectInputStream in = new ObjectInputStream(fileIn);
            deserialized = (CLAIDSpecialPermissionsConfig) in.readObject();
            in.close();
            fileIn.close();
            return deserialized;
        } 
        catch (IOException i) 
        {
            i.printStackTrace();
            return null;
        } 
        catch (ClassNotFoundException c) 
        {
            c.printStackTrace();
            return null;
        }
    }


}
