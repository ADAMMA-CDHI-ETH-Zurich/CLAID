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
import adamma.c4dhi.claid_android.CLAIDServices.ServiceManager;
import adamma.c4dhi.claid_android.CLAIDServices.CLAIDService;
import adamma.c4dhi.claid_android.Permissions.*;

import android.content.Context;
import android.app.Application;

import android.content.res.AssetManager;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.IllegalArgumentException;


import android.content.Intent;
import android.os.Build;
import android.os.PowerManager;
import android.provider.Settings;
import android.app.AlertDialog;
import android.content.DialogInterface;

public class CLAID extends JavaCLAIDBase
{
    static
    {
        init("claid_capi_android");
    }

    private static Context context;
    private static AsyncRunnablesHelperThread asyncRunnablesHelperThread;
    
    // ModuleFactory that is used when CLAID is started in an Android Service (e.g., the MaximumPermissionsPerpetualService).
    // Since the Service can crash and be restarted, the ModuleFactory needs to survive a complete restart.
    // For this, the ModuleFactory cannot be populated from an Activity, as the Activity will not be restarted automatically after the 
    // service is restarted. The PersistentModuleFactory therefore has to be populated from the Application's onCreate function. 
    // The PersistentModuleFactory can only be created by passing an instance of Application to the constructor, forcing the user 
    // to register Modules only when a custom Application class is available.
    private static PersistentModuleFactory persistentModuleFactory = null;
    
    // Starts the middleware and attaches to it.
    public static boolean start(Context context, final String socketPath, final String configFilePath, final String hostId, final String userId, final String deviceId, ModuleFactory moduleFactory)
    {
        CLAID.context = context;
    
        String adjustedConfigPath = configFilePath;

        if(configFilePath.startsWith("assets://"))
        {
          
            String appDataDirPath = getAppDataDirectory(context);

            String tmpConfigPath = appDataDirPath + "/claid_config.json";

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

   
        // Get the absolute path to the app's data directory
        String appDataDirPath = getAppDataDirectory(context);

        String socketPath = "unix://" + appDataDirPath + "/claid_local.grpc";

        return start(context, socketPath, configFilePath, hostId, userId, deviceId, moduleFactory);
    }

    public static boolean startInPersistentService(Context context, final String configFilePath, final String hostId, final String userId, final String deviceId, PersistentModuleFactory moduleFactory)
    {
        String appDataDirPath = getAppDataDirectory(context);

        String socketPath = "unix://" + appDataDirPath + "/claid_local.grpc";

        return startInPersistentService(context, socketPath, configFilePath, hostId, userId, deviceId, moduleFactory);
    }

    public static boolean startInPersistentService(Context context, final String socketPath, final String configFilePath, final String hostId, final String userId, final String deviceId, PersistentModuleFactory moduleFactory)
    {
        if(moduleFactory != CLAID.persistentModuleFactory)
        {
            Logger.logError("Failed to start CLAID in persistent service. The provided PersistentModuleFactory has not been created with CLAID.getPersistentModuleFactory()." +
            "Use CLAID.getPersistentModuleFactory() to retrieve a PersistentModuleFactory, and then pass ist to CLAID.startInPersistentService(...), after you have registered your Module classes.");
        }
      
        CLAID.context = context;

        

        // Start service from a separate thread.
        // The ServiceManager might need to request permissions, which has to be done in a separate thread when using CLAID's Permission classes.
        
        if(asyncRunnablesHelperThread == null)
        {
            asyncRunnablesHelperThread = new AsyncRunnablesHelperThread();
        }
        if(!asyncRunnablesHelperThread.isRunning())
        {
            if(!asyncRunnablesHelperThread.start())
            {
                return false;
            }
        }

        asyncRunnablesHelperThread.insertRunnable(() ->
            ServiceManager.startMaximumPermissionsPerpetualService(context, socketPath, configFilePath, hostId, userId, deviceId));
        
        return true;
    }

    public static<T extends Application> PersistentModuleFactory getPersistentModuleFactory(T application)
    {
        if(CLAID.persistentModuleFactory != null)
        {
            if(CLAID.persistentModuleFactory.getApplication() != application)
            {
                Logger.logError("Failed to getPersistentModuleFactory. A PersistentModuleFactory was already registered for CLAID but using another instance of application.\n" + 
                            "The original application instance was of class \"" + CLAID.persistentModuleFactory.getApplication().getClass().getName() + "\", instance " + CLAID.persistentModuleFactory.getApplication() + "." +
                            "Afterward, getPersistentModuleFactory was called again with an application of class \"" + application.getClass().getName() + "\", instance " + application + ".\n" + 
                            "Make sure to only retrieve the PersistentModuleFactory once, or use the exact same Application Object in both cases.");
                System.exit(0);
            }
            return CLAID.persistentModuleFactory;
        }

        try
        {
            CLAID.persistentModuleFactory = new PersistentModuleFactory(application);
        }
        catch(IllegalArgumentException e)
        {
            Logger.logError(e.getMessage());
            e.printStackTrace();
            System.exit(0);
        }

        return CLAID.persistentModuleFactory;
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

    public static void onServiceStarted(CLAIDService service, final String socketPath, final String configFilePath, final String hostId, final String userId, final String deviceId)
    {
        if(CLAID.persistentModuleFactory == null)
        {
            Logger.logError("Failed to run CLAID in CLAIDService. PersistentModuleFactory is null, so CLAID.getPersistentModuleFactory() was not called before.\n" + 
                            "In order to run CLAID in a Service, you need to call getPersistentModuleFactory() in your Application's (NOT Activity) onCreate function.\n" +
                            "This is necessary to ensure that the PersistentModuleFactory is populated before the Service is started.");
            System.exit(0);
        }

        if(!CLAID.start((Context) service, socketPath, configFilePath, hostId, userId, deviceId, CLAID.persistentModuleFactory))
        {
            Logger.logError("Failed to start CLAID in CLAIDService. CLAID.start failed. Previous logs should indicate the reason.");
            System.exit(0);
        }
    }

    private static Thread permissionHelperThread;

    public static boolean hasMicrophonePermission()
    {
        Logger.logInfo("has microphone permission");
        return new MicrophonePermission().isGranted();
    }

    public static boolean requestMicrophonePermission()
    {
        Logger.logInfo("request microphone permission");

        MicrophonePermission permission = new MicrophonePermission();
        Logger.logInfo("request microphone permission 2");

        permission.blockingRequest();
        Logger.logInfo("request microphone permission 3");

        return permission.isGranted();
    }

    public static boolean hasLocationPermission()
    {
        return new LocationPermission().isGranted();
    }

    public static boolean requestLocationPermission()
    {
        LocationPermission permission = new LocationPermission();
        permission.blockingRequest();

        return permission.isGranted();
    }

    public static boolean hasStoragePermission()
    {
        return new StoragePermission().isGranted();
    }

    public static boolean requestStoragePermission()
    {
        StoragePermission permission = new StoragePermission();
        permission.blockingRequest();

        return permission.isGranted();
    }

    public static void onUnrecoverableException(final String exception)
    {
        Logger.logFatal(exception);
        throw new RuntimeException(exception);
        // android.os.Process.killProcess(android.os.Process.myPid());
        // System.exit(0);
    }

    public static boolean isBatteryOptimizationDisabled(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            PowerManager powerManager = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
            return powerManager.isIgnoringBatteryOptimizations(context.getPackageName());
        }
        // For versions prior to Android M, battery optimization is not applicable.
        return true;
    }

    public static void requestBatteryOptimizationExemption(Context context) {
        
        if(isBatteryOptimizationDisabled(context))
        {
            return;
        }



        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {

            if(isCLAIDContextAnActivity())
            {
                android.app.Activity activity = (android.app.Activity) CLAID.getContext();
                
                activity.runOnUiThread(() ->
                    new AlertDialog.Builder(context)
                    .setMessage("On the next page, please disable battery optimizations. This is required to make CLAID run in the background as long as possible.")
                    .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            showBatteryOptimizationIntent(context);
                        }
                    })
                    .show());
            }
            else
            {
                showBatteryOptimizationIntent(context);
            }

            
        

            
        }
    }

    private static void showBatteryOptimizationIntent(Context context)
    {
        Intent intent = new Intent();
        String packageName = context.getPackageName();
        PowerManager powerManager = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
    

        if (!powerManager.isIgnoringBatteryOptimizations(packageName)) {
            intent.setAction(Settings.ACTION_REQUEST_IGNORE_BATTERY_OPTIMIZATIONS);
            intent.setData(android.net.Uri.parse("package:" + packageName));
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent);
        }
    }

    public static boolean isCLAIDContextAnActivity()
    {
        if(CLAID.context == null)
        {
            return false;
        }

        return (CLAID.context instanceof android.app.Activity);
    }

    public static String getAppDataDirectory(Context context)
    {
        File appDataDir = context.getFilesDir();

        // Get the absolute path to the app's data directory
        String appDataDirPath = appDataDir.getAbsolutePath();
        return appDataDirPath;
    }

    // Implement functions for context management etc.
}
