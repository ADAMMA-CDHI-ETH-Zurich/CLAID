/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Stephan Altmüller
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

package adamma.c4dhi.claid_platform_impl;

import adamma.c4dhi.claid.JavaCLAIDBase;
import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid.Logger.Logger;

import android.content.Context;

import android.content.res.AssetManager;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;


public class CLAID extends JavaCLAIDBase
{
    static
    {
        init("claid_capi_android");
    }

    private static Context context;
    
    // Starts the middleware and attaches to it.
    public static boolean start(Context context, final String socketPath, final String configFilePath, final String hostId, final String userId, final String deviceId, ModuleFactory moduleFactory)
    {
        CLAID.context = context;
    
        String adjustedConfigPath = configFilePath;

        if(configFilePath.startsWith("assets://"))
        {
            File appDataDir = context.getFilesDir();

            // Get the absolute path to the app's data directory
            String appDataDirPath = appDataDir.getAbsolutePath();

            String tmpConfigPath = appDataDir + "/claid_config.json";

            String assetFileName = configFilePath.substring("assets://".length());
            if(!copyFileFromAssetsToPath(context, assetFileName, tmpConfigPath))
            {
                return false;
            }

            adjustedConfigPath = tmpConfigPath;
        }

        return startInternal(socketPath, adjustedConfigPath, hostId, userId, deviceId, moduleFactory);
    }

    // Starts the middleware and attaches to it.
    public static boolean start(Context context, final String configFilePath, final String hostId, final String userId, final String deviceId, ModuleFactory moduleFactory)
    {
        CLAID.context = context;

        File appDataDir = context.getFilesDir();

        // Get the absolute path to the app's data directory
        String appDataDirPath = appDataDir.getAbsolutePath();

        String socketPath = "unix://" + appDataDirPath + "/claid_local.grpc";

        return start(context, socketPath, configFilePath, hostId, userId, deviceId, moduleFactory);
    }

    // Attaches to the Middleware, but does not start it.
    // Assumes that the middleware is started in another language (e.g., C++ or Dart).
    // HAS to be called AFTER start is called in ANOTHER language.
    public static boolean attachJavaRuntime(Context context, final String socketPath, ModuleFactory factory)
    {
        CLAID.context = context;
        return attachJavaRuntimeInternal(socketPath, factory);
    }

    public static boolean attachJavaRuntime(Context context, long handle, ModuleFactory factory)
    {
        CLAID.context = context;
        return attachJavaRuntimeInternal(handle, factory);
    }

    public static Context getContext()
    {
        return CLAID.context;
    }

    private static boolean copyFileFromAssetsToPath(Context context, String assetFileName, String destinationPath) 
    {
        AssetManager assetManager = context.getAssets();

        try 
        {
            // Open the asset file
            InputStream inputStream = assetManager.open(assetFileName);

            // Create the destination file
            File destinationFile = new File(destinationPath);

            // Create a stream to write the data to the destination file
            OutputStream outputStream = new FileOutputStream(destinationFile);

            // Transfer bytes from the input stream to the output stream
            byte[] buffer = new byte[1024];
            int length;
            while ((length = inputStream.read(buffer)) > 0) {
                outputStream.write(buffer, 0, length);
            }

            // Close the streams
            inputStream.close();
            outputStream.close();
            return true;
        } 
        catch (IOException e) 
        {
            Logger.logError("Failed to copy assets file \"" + assetFileName + "\" to path \"" + destinationPath + "\":" + e.getMessage());
            e.printStackTrace();
            return false;
            // Handle the exception as needed
        }
    }

    // Implement functions for context management etc.
}
