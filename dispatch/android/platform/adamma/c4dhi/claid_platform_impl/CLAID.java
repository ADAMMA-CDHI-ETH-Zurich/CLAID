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
import adamma.c4dhi.claid_android.Configuration.CLAIDPersistanceConfig;
import adamma.c4dhi.claid_android.Configuration.CLAIDSpecialPermissionsConfig;
import adamma.c4dhi.claid_android.CLAIDServices.CLAIDService;
import adamma.c4dhi.claid_android.Permissions.*;
import adamma.c4dhi.claid_android.Receivers.DeviceOwnerReceiver;
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
import android.os.PowerManager;

import android.os.Build;

import android.content.pm.PackageManager;

import static android.content.Context.POWER_SERVICE;

import android.app.admin.DevicePolicyManager;
import android.content.ComponentName;
import android.content.Context;
import android.os.Build;
import android.os.PersistableBundle;
import android.os.UserManager;


import android.bluetooth.BluetoothAdapter;
import android.net.wifi.WifiManager;

import android.Manifest;
import androidx.core.app.ActivityCompat;


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
    
    private static PowerManager.WakeLock claidWakeLock;

    // Starts the middleware and attaches to it.
    // Volatile, as CLAID might only run as long as the App is in the foreground.
    public static boolean start(Context context, final String socketPath, 
        final String configFilePath, final String hostId, final String userId, 
        final String deviceId, ModuleFactory moduleFactory, CLAIDSpecialPermissionsConfig specialPermissionsConfig)
    {
        CLAID.context = context;
    
        String adjustedConfigPath = configFilePath;

        if(specialPermissionsConfig != null)
        {
            if(!CLAID.grantSpecialPermissions(context, specialPermissionsConfig))
            {
                Logger.logWarning("CLAID.start() failed: CLAID.grantSpecialPermissions() not successfull.");
                return false;
            }
        }

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
    // Volatile, as CLAID might only run as long as the App is in the foreground.
    public static boolean start(Context context, final String socketPath, 
        final String configFilePath, final String hostId, final String userId, 
        final String deviceId, ModuleFactory moduleFactory)
    {
        return start(context, socketPath, configFilePath, hostId, userId, deviceId, moduleFactory, null);
    }



    // Starts the middleware and attaches to it.
    // Volatile, as CLAID might only run as long as the App is in the foreground.
    public static boolean start(Context context, final String configFilePath, 
        final String hostId, final String userId, final String deviceId, 
        ModuleFactory moduleFactory, CLAIDSpecialPermissionsConfig specialPermissionsConfig)
    {
        CLAID.context = context;

   
        // Get the absolute path to the app's data directory
        String appDataDirPath = getAppDataDirectory(context);

        String socketPath = "unix://" + appDataDirPath + "/claid_local.grpc";

        return start(context, socketPath, configFilePath, hostId, userId, deviceId, moduleFactory, specialPermissionsConfig);
    }

    public static boolean startInPersistentService(Context context, final String configFilePath, 
        final String hostId, final String userId, final String deviceId, 
        PersistentModuleFactory moduleFactory, CLAIDSpecialPermissionsConfig specialPermissionsConfig, CLAIDPersistanceConfig persistanceConfig)
    {
        String appDataDirPath = getAppDataDirectory(context);

        String socketPath = "unix://" + appDataDirPath + "/claid_local.grpc";

        return startInPersistentService(context, socketPath, configFilePath, hostId, userId, deviceId, moduleFactory, specialPermissionsConfig, persistanceConfig);
    }

    public static boolean startInPersistentService(Context context, final String socketPath, 
        final String configFilePath, final String hostId, final String userId, 
        final String deviceId, PersistentModuleFactory moduleFactory, CLAIDSpecialPermissionsConfig specialPermissionsConfig, CLAIDPersistanceConfig enduranceConfig)
    {
        if(moduleFactory != CLAID.persistentModuleFactory)
        {
            Logger.logError("Failed to start CLAID in persistent service. The provided PersistentModuleFactory has not been created with CLAID.getPersistentModuleFactory()." +
            "Use CLAID.getPersistentModuleFactory() to retrieve a PersistentModuleFactory, and then pass ist to CLAID.startInPersistentService(...), after you have registered your Module classes.");
        }
      
        CLAID.context = context;

        if(!CLAID.grantSpecialPermissions(context, specialPermissionsConfig))
        {
            Logger.logWarning("CLAID.startInPersistentService() failed: CLAID.grantSpecialPermissions() not successfull.");
            return false;
        }

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
            ServiceManager.startMaximumPermissionsPerpetualService(context, socketPath, configFilePath, hostId, userId, deviceId, enduranceConfig));
        
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

    public static void onServiceStarted(CLAIDService service, final String socketPath, 
        final String configFilePath, final String hostId, final String userId, final String deviceId)
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

    private static boolean grantSpecialPermissions(Context context, CLAIDSpecialPermissionsConfig specialPermissionsConfig)
    {
        Thread thread = new Thread(() -> {
            if(specialPermissionsConfig.MANAGE_ALL_STORAGE && !CLAID.hasStoragePermission())
            {
                if(!CLAID.requestStoragePermission())
                {
                    Logger.logError("CLAID grantSpecialPermissions failed. Could not get full storage permissions");
                    System.exit(0);
                }
            }

            if(specialPermissionsConfig.DISABLE_BATTERY_OPTIMIZATIONS && !CLAID.isBatteryOptimizationDisabled(context))
            {
                CLAID.requestBatteryOptimizationExemption(context); 
            }
            Logger.logInfo("Mightiness config: " + specialPermissionsConfig.BE_DEVICE_OWNER);

            if(specialPermissionsConfig.BE_DEVICE_OWNER && !CLAID.DeviceOwnerFeatures.isDeviceOwner(context))
            {
                Logger.logInfo("Requesting device ownership");
                CLAID.DeviceOwnerFeatures.requestDeviceOwnership(context);
            }
        });

        thread.start();
        
        return true;
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

    public static boolean hasNotificationPermission()
    {
        return new NotificationPermission().isGranted();
    }

    public static boolean requestNotificationPermission()
    {
        NotificationPermission permission = new NotificationPermission();
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
        Logger.logInfo("Checking battery optimization 1");

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            Logger.logInfo("Checking battery optimizatio2");

            PowerManager powerManager = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
            Logger.logInfo("Checking battery optimization 3 " + context.getPackageName());

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

            if(!CLAID.isWearOS(context))
            {
                if(context instanceof android.app.Activity)
                {
                    android.app.Activity activity = (android.app.Activity) context;
                    
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
            else
            {
                String batteryOptmizationMessage = 
                "For CLAID services to run as long as possible and being able to restart silently from the background, it is necessary\n" +
                "to disable battery optimizations for this App. On WearOS, however, it is currently not possible " + 
                "to request the user to disable battery optimizations (in contrast to regular Android).\n" +
                "On WearOS, battery optimizations can currently ONLY be disabled using ADB (Android Debug Bridge).\n" + 
                "Connect ADB to your device and enter the following command: \"adb shell dumpsys deviceidle whitelist +" + context.getPackageName() + "\".\n" +
                "Restart the App afterwards, and battery optimizations should be disabled. You can verify this using CLAID.isBatteryOptimizationDisabled().";

                if(context instanceof android.app.Activity)
                {
                    android.app.Activity activity = (android.app.Activity) context;
                    
                    activity.runOnUiThread(() ->
                        new AlertDialog.Builder(context)
                        .setMessage(batteryOptmizationMessage)
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
                    Logger.logError("Failed to show message with instructions on how to remove battery optimizations on WearOS to the user. The context is not an Activity.");
                }
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

    public static File getMediaDirPath(Context context)
    {
        File[] files = context.getExternalMediaDirs();

        for(File f : files)
        {
            Logger.logInfo("CLAID_FILE " + f);

        }
        return files[0];
    }



    // This function registers a WakeLock, which will prevent the
    // device from going to sleep/doze. 
    // This is mainly relevant for WearOS applications,
    // which go into doze very quickly/aggressively, even if a Service is running.
    // On regular Smartphones, this is mostly not necessary if a Service is used.
    public static boolean enableKeepAppAwake()
    {  
        if(claidWakeLock == null)
        {
            if(context == null)
            {
                return false;
            }
            PowerManager powerManager = (PowerManager) context.getSystemService(POWER_SERVICE);
            claidWakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                    "CLAID::PartialWakeLock");
        }

        if(claidWakeLock.isHeld())
        {
            return true;
        }

        claidWakeLock.acquire();
        return true;
    }

    public static boolean disableKeepAppAwake()
    {
        if(claidWakeLock == null)
        {
            return true;
        }

        if(!claidWakeLock.isHeld())
        {
            return true;
        }

        claidWakeLock.release();
        return true;
    }

    public static boolean disableKeepAppAwake(Integer waitTimeMs)
    {
        try
        {
            Thread.sleep(waitTimeMs);
        }
        catch(InterruptedException e)
        {
            e.printStackTrace();
        }

        return disableKeepAppAwake();
    }

    public static boolean isWearOS(Context context) {
        // Check if the device is running Wear OS
        PackageManager pm = context.getPackageManager();
        return pm.hasSystemFeature(PackageManager.FEATURE_WATCH);
    }



    public static class DeviceOwnerFeatures
    {
        
        public static boolean isDeviceOwner(Context context)
        {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                DevicePolicyManager dpm = (DevicePolicyManager) context.getSystemService(Context.DEVICE_POLICY_SERVICE);
                ComponentName adminComponent = new ComponentName(context, DeviceOwnerReceiver.class);
                
                // Check if the app is a device owner
                return dpm.isDeviceOwnerApp(context.getPackageName());
                
                // Alternatively, you can check if the component is an active admin
                // return dpm.isAdminActive(adminComponent);
            } else {
                // For versions below M, the concept of device owner doesn't exist.
                return false;
            }
        }

        public static boolean enableBluetooth(Context context)
        {
            if(!DeviceOwnerFeatures.isDeviceOwner(context))
            {
                Logger.logWarning("CLAID cannot enable bluetooth. We are not a DeviceManager");
                return false;
            }
            BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

            if (bluetoothAdapter.isEnabled()) 
            {
                // Already enabled, return true.
                return true;
            }
            if (ActivityCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) 
            {                        
                Logger.logWarning("CLAID cannot enable bluetooth. We do no thave BLUETOOTH_CONNECT permission.");
                return false;
            }
            return bluetoothAdapter.enable();
        }

        public static boolean disableBluetooth(Context context)
        {
            if(!DeviceOwnerFeatures.isDeviceOwner(context))
            {
                Logger.logWarning("CLAID cannot disable bluetooth. We are not a DeviceManager");
                return false;
            }
            BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

            if (!bluetoothAdapter.isEnabled()) 
            {
                // Already disabled, return true.
                return true;
            }
            if (ActivityCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) 
            {                        
                Logger.logWarning("CLAID cannot disable bluetooth. We do no thave BLUETOOTH_CONNECT permission.");
                return false;
            }
            return bluetoothAdapter.disable();
        }

        public static boolean enableWifi(Context context)
        {
            Logger.logInfo("CLAID enable wifi called");
            if(!DeviceOwnerFeatures.isDeviceOwner(context))
            {
                Logger.logWarning("CLAID cannot enable bluetooth. We are not a DeviceManager");
                return false;
            }
            
            WifiManager wifi = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        
            if(wifi.isWifiEnabled())
            {
                return true;
            }

            return wifi.setWifiEnabled(true);
        }

        public static boolean disableWifi(Context context)
        {
            Logger.logInfo("CLAID disable wifi called");
            if(!DeviceOwnerFeatures.isDeviceOwner(context))
            {
                Logger.logWarning("CLAID cannot disable wifi. We are not a DeviceManager");
                return false;
            }
            
            WifiManager wifi = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        
            if(!wifi.isWifiEnabled())
            {
                return true;
            }

            return wifi.setWifiEnabled(false);
        }

        public static void requestDeviceOwnership(Context context)
        {
            String batteryOptmizationMessage = 
                "CLAID was started with option BE_DEVICE_OWNER in the MIGHTINESS configuration.\n" +
                "In order to continue, the application needs to be registered as device owner, which allows it to have more control\n" +
                "over the operating system and to have additional permissions (e.g., to disable or enable wifi from the background)\n." +
                "You can find the instructions on how to register this application as device owner below.\n" +
                "Before you proceed: Please note that device owner Apps can NOT be stopped and NOT be uninstalled.\n" +
                "If you want to stop or uninstall the application, you have to remove the device ownership first (see below).\n\n"+
                "To register this application as device owner, you have to do the following:\n" + 
                "1st: Remove all existing google accounts from this device (check \"accounts\" page in the settings)." + 
                "2nd: Connect ADB (Android Debug Bridge) to your device and execute the following command: \"adb shell dpm set-device-owner " + context.getPackageName() + "/adamma.c4dhi.claid_android.Receivers.DeviceOwnerReceiver\n" +
                "\n\n\nTo remove device ownership at a later point, execute the following command: \"adb remove-active-admin " + context.getPackageName() + "/adamma.c4dhi.claid_android.Receivers.DeviceOwnerReceiver\n";

                

                if(context instanceof android.app.Activity)
                {
                    android.app.Activity activity = (android.app.Activity) context;
                    
                    activity.runOnUiThread(() ->
                        new AlertDialog.Builder(context)
                        .setMessage(batteryOptmizationMessage)
                        .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                System.exit(0);
                            }
                        })
                        .show());
                }
                else
                {
                    Logger.logError("Failed to show message with instructions on how to enable device ownership to the user. The context is not an Activity.");
                }
        }
    }


}
