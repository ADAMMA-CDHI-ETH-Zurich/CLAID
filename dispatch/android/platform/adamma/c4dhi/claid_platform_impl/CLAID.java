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

package adamma.c4dhi.claid_platform_impl;

import adamma.c4dhi.claid.JavaCLAIDBase;
import adamma.c4dhi.claid.Module.Module;

import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid_android.CLAIDServices.ServiceManager;
import adamma.c4dhi.claid_android.Configuration.CLAIDPersistanceConfig;
import adamma.c4dhi.claid_android.Configuration.CLAIDSpecialPermissionsConfig;
import adamma.c4dhi.claid_android.BatteryManagement.BatterySaverModule;
import adamma.c4dhi.claid_android.CLAIDServices.CLAIDService;
import adamma.c4dhi.claid_android.Permissions.*;
import adamma.c4dhi.claid_android.Receivers.DeviceOwnerReceiver;
import adamma.c4dhi.claid_android.UserFeedback.TextToSpeechModule;
import android.content.Context;
import android.app.Application;

import android.location.LocationManager;

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
import adamma.c4dhi.claid_android.collectors.battery.AlarmBatteryCollector;
import adamma.c4dhi.claid_android.collectors.audio.MicrophoneCollector;
import adamma.c4dhi.claid_android.collectors.battery.BatteryCollector;
import adamma.c4dhi.claid_android.collectors.heartrate.HeartRateCollector;
import adamma.c4dhi.claid_android.collectors.motion.AccelerometerCollector;
import adamma.c4dhi.claid_android.collectors.motion.GyroscopeCollector;
import adamma.c4dhi.claid_android.collectors.location.LocationCollector;
import adamma.c4dhi.claid_android.collectors.audio.MicrophoneCollector;

import adamma.c4dhi.claid_android.UserFeedback.NotificationModule;
import adamma.c4dhi.claid_android.UserFeedback.TextToSpeechModule;
import adamma.c4dhi.claid_android.BatteryManagement.BatterySaverModule;
import adamma.c4dhi.claid_android.Dialog.Dialog;
import adamma.c4dhi.claid_android.Package.CLAIDPackage;
import adamma.c4dhi.claid_android.Package.CLAIDPackageLoader;

import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.os.Bundle;
import android.os.PowerManager;
import android.provider.Settings;
import android.view.WindowManager;
import android.app.Activity;
import java.util.List;

public class CLAID extends JavaCLAIDBase
{
    static
    {
        init("claid_capi_android");
    }

    private static Context context;
    private static AsyncRunnablesHelperThread asyncRunnablesHelperThread;
    
    private static ModuleFactory claidGlobalModuleFactory = new ModuleFactory();
    
    private static PowerManager.WakeLock claidWakeLock;
    // This counter is increased when the wake lock is acquired, and decreased when it is released.
    // Once the counter reaches 0, the wake lock is released, meaning that all entities/Modules
    // having acquired the wake lock have released it again.
    private static int wakeLockCounter = 0;

    private static Runnable onCLAIDStartedCallback = null;
    
    private static Thread claidThread = null;

    public static boolean registerModule(Class<? extends Module> clz)
    {
        return CLAID.claidGlobalModuleFactory.registerModule(clz);
    }

    // Starts the middleware and attaches to it.
    // Volatile, as CLAID might only run as long as the App is in the foreground.
    private static boolean start(Context context, final String socketPath, 
        final String configFilePath, final String hostId, final String userId, 
        final String deviceId, CLAIDSpecialPermissionsConfig specialPermissionsConfig)
    {
        CLAID.context = context;
        
        String adjustedConfigPath = configFilePath;

        if(specialPermissionsConfig != null)
        {
            if(!CLAID.grantSpecialPermissions(context, specialPermissionsConfig))
            {
                Logger.logError("CLAID.start() failed: CLAID.grantSpecialPermissions() not successfull.");
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

        CLAIDPackageLoader.loadPackages(context);

        boolean result = startInternalWithEventTracker(socketPath, adjustedConfigPath, 
            hostId, userId, deviceId, CLAID.claidGlobalModuleFactory, CLAID.getMediaDirPath(context).toString());

        if(result) 
        {
            if(onCLAIDStartedCallback != null)
            {
                onCLAIDStartedCallback.run();
            }
        }
        return result;
    
    }

    // Starts the middleware and attaches to it.
    // Volatile, as CLAID might only run as long as the App is in the foreground.
    private static boolean start(Context context, final String socketPath, 
        final String configFilePath, final String hostId, final String userId, 
        final String deviceId)
    {
        return start(context, socketPath, configFilePath, hostId, userId, deviceId, null);
    }



    // Starts the middleware and attaches to it.
    // Volatile, as CLAID might only run as long as the App is in the foreground.
    private static boolean start(Context context, final String configFilePath, 
        final String hostId, final String userId, final String deviceId, 
        CLAIDSpecialPermissionsConfig specialPermissionsConfig)
    {
        CLAID.context = context;

   

        if(asyncRunnablesHelperThread == null)
        {
            asyncRunnablesHelperThread = new AsyncRunnablesHelperThread();
        }
        if(!asyncRunnablesHelperThread.isRunning())
        {
            if(!asyncRunnablesHelperThread.start())
            {
                Logger.logError("Failed to start async runnables helper thread");
                return false;
            }
        }

        // Get the absolute path to the app's data directory
        String appDataDirPath = getAppDataDirectory(context);

        String socketPath = "unix://" + appDataDirPath + "/claid_local.grpc";

        return start(context, socketPath, configFilePath, hostId, userId, deviceId, specialPermissionsConfig);
    }

    public static boolean startInForeground(Context context, String socketPath,
        final String configFilePath, final String hostId, final String userId, 
        final String deviceId, CLAIDSpecialPermissionsConfig specialPermissionsConfig)
    {
        ServiceManager.stopService(context);
        return CLAID.start(context, socketPath, configFilePath, hostId, userId, deviceId, specialPermissionsConfig);
    }

    public static boolean startInForeground(Context context, 
        final String configFilePath, final String hostId, final String userId, 
        final String deviceId, CLAIDSpecialPermissionsConfig specialPermissionsConfig)
    {
        ServiceManager.stopService(context);
        return CLAID.start(context, configFilePath, hostId, userId, deviceId, specialPermissionsConfig);
    }

    public static boolean startInBackground(Context context, final String configFilePath, 
        final String hostId, final String userId, final String deviceId, 
        CLAIDSpecialPermissionsConfig specialPermissionsConfig, CLAIDPersistanceConfig persistanceConfig)
    {
        String appDataDirPath = getAppDataDirectory(context);

        String socketPath = "unix://" + appDataDirPath + "/claid_local.grpc";

        return startInBackground(context, socketPath, configFilePath, hostId, userId, deviceId, specialPermissionsConfig, persistanceConfig);
    }

    public static boolean startInBackground(Context context, final String socketPath, 
        final String configFilePath, final String hostId, final String userId, 
        final String deviceId, CLAIDSpecialPermissionsConfig specialPermissionsConfig, CLAIDPersistanceConfig enduranceConfig)
    {
        if(ServiceManager.isServiceRunning())
        {
            return true;
        }
      
        CLAID.context = context;

        if(!CLAID.grantSpecialPermissions(context, specialPermissionsConfig))
        {
            Logger.logWarning("CLAID.startInBackground() failed: CLAID.grantSpecialPermissions() not successfull.");
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

    public static void onStarted(Runnable runnable)
    {
        CLAID.onCLAIDStartedCallback = runnable;
        if(CLAID.isRunning())
        {
            runnable.run();
        }
    }

    public static ModuleFactory registerDefaultModulesToFactory(ModuleFactory factory)
    {
        factory.registerModule(BatteryCollector.class);
        factory.registerModule(AccelerometerCollector.class);
        factory.registerModule(GyroscopeCollector.class);
        factory.registerModule(NotificationModule.class);
        factory.registerModule(TextToSpeechModule.class);
        factory.registerModule(HeartRateCollector.class);
        factory.registerModule(BatterySaverModule.class);
        factory.registerModule(LocationCollector.class);
        factory.registerModule(MicrophoneCollector.class);
        factory.registerModule(AlarmBatteryCollector.class);

        return factory;
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
        if(!CLAID.start((Context) service, socketPath, configFilePath, hostId, userId, deviceId))
        {
            Logger.logError("Failed to start CLAID in CLAIDService. CLAID.start failed. Previous logs should indicate the reason.");
            System.exit(0);
        }
    }

    // In plain Java, we simply stop the Middleware.
    // In Android, we also might have to stop the services, so this might be implemented differently
    public static void shutdown()
    {
        shutdownInternal();   
        if(ServiceManager.isServiceRunning())
        {
            ServiceManager.stopService(CLAID.getContext());
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

    public static boolean requestPermissions(String[] manifestPermissionNames, String dialogMessage)
    {
        GenericPermission genericPermission = new GenericPermission(manifestPermissionNames, dialogMessage);
        genericPermission.blockingRequest();

        return genericPermission.isGranted();
    }

    public static boolean requestPermissionsSequential(String[] manifestPermissionNames, String dialogMessage)
    {
        for(String permissionName : manifestPermissionNames)
        {
            GenericPermission genericPermission = new GenericPermission(new String[]{permissionName}, dialogMessage);
            genericPermission.blockingRequest();

            if(!genericPermission.isGranted())
            {
                return false;
            }
        }
        return true;
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

    public static boolean isAppOnForeground(Context context) {
        ActivityManager activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> appProcesses = activityManager.getRunningAppProcesses();
        if (appProcesses == null) {
            return false;
        }
        final String packageName = context.getPackageName();
        for (ActivityManager.RunningAppProcessInfo appProcess : appProcesses) {
            if (appProcess.importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_FOREGROUND && appProcess.processName.equals(packageName)) {
                return true;
            }
        }
        return false;
    }

    public static void onUnrecoverableException(final String exception)
    {
        Logger.logFatal(exception);
        throw new RuntimeException(exception);
        // android.os.Process.killProcess(android.os.Process.myPid());
        // System.exit(0);
    }

    public static boolean isBatteryOptimizationDisabled(Context context) 
    {
        BatteryOptimizationExemption exemption = new BatteryOptimizationExemption();

        return exemption.isGranted();
    }

    public static void requestBatteryOptimizationExemption(Context context) 
    { 
        BatteryOptimizationExemption exemption = new BatteryOptimizationExemption();
        if(!exemption.isGranted())
        {
            exemption.blockingRequest();
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
    public static boolean acquireWakeLock(Context context)
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

        wakeLockCounter++;

        if(claidWakeLock.isHeld())
        {
            return true;
        }

        Logger.logWarning("Wakelock enabled");
        claidWakeLock.acquire();
        return true;
    }

    public static boolean releaseWakeLock(Context context)
    {
        if(claidWakeLock == null)
        {
            return true;
        }
        wakeLockCounter--;
        if(wakeLockCounter <= 0)
        {
            wakeLockCounter = 0;
            if(!claidWakeLock.isHeld())
            {
                return true;
            }
            claidWakeLock.release();
        }

        return true;
    }

    private static void releaseWakeLock(Context context, Integer waitTimeMs)
    {
        try
        {
            Thread.sleep(waitTimeMs);
        }
        catch(InterruptedException e)
        {
            e.printStackTrace();
        }

        Logger.logWarning("WakeLock disabled");
        releaseWakeLock(context);
    }

    public static void releaseWakeLockAfterMs(Context context, Integer waitTimeMs)
    {
        asyncRunnablesHelperThread.insertRunnable(() ->releaseWakeLock(context, waitTimeMs));
    }

    public static boolean isWearOS(Context context) {
        // Check if the device is running Wear OS
        PackageManager pm = context.getPackageManager();
        return pm.hasSystemFeature(PackageManager.FEATURE_WATCH);
    }

    public static void displayAlertDialog(String title, String body)
    {
        Dialog dialog = new Dialog();
        dialog.showDialogBlocking(title, body, null);
    }

    public static void displayAlertDialog(String title, String body, Runnable runnable)
    {
        Dialog dialog = new Dialog();
        dialog.showDialogBlocking(title, body, runnable);
    }


    public static boolean isBluetoothEnabled()
    {
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        if(bluetoothAdapter == null)
        {
            return false;
        }

        return bluetoothAdapter.isEnabled();
    }

    public static boolean isLocationEnabled()
    {
        LocationManager locationManager = (LocationManager) CLAID.getContext().getSystemService(Context.LOCATION_SERVICE);
        if (locationManager == null) 
        {
            return false;
        }
        boolean isLocationEnabled = locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER) || locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
        return isLocationEnabled;
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
            String deviceOwnershipMessage = 
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

                

            CLAID.displayAlertDialog("Enable device ownership", deviceOwnershipMessage);
                
        }
    }



}
