package adamma.c4dhi.claid;
import android.os.Bundle;

import androidx.annotation.Nullable;

import java.util.List;

import adamma.c4dhi.claid_platform_impl.CLAID;
import io.flutter.embedding.android.FlutterActivity;
import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.plugin.common.MethodChannel;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid_android.Configuration.CLAIDPersistanceConfig;
import adamma.c4dhi.claid_android.Configuration.CLAIDSpecialPermissionsConfig;


import io.flutter.embedding.android.FlutterActivity;
import io.flutter.embedding.android.FlutterSurfaceView;
import io.flutter.embedding.engine.FlutterEngine;
import io.flutter.plugin.common.MethodChannel;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class CLAIDFlutterActivity extends FlutterActivity
{
    private static final String CHANNEL = "adamma.c4dhi.claid/FLUTTERCLAID";
    private static boolean backgroundStarted = false;
    @Override
    public void configureFlutterEngine(FlutterEngine flutterEngine) {
        super.configureFlutterEngine(flutterEngine);
        new MethodChannel(flutterEngine.getDartExecutor().getBinaryMessenger(), CHANNEL)
                .setMethodCallHandler(
                        (call, result) -> 
                        {
                            Logger.logInfo("Platform channel function called " + call.method);
                            if (call.method.equals("startInForeground")) 
                            {
                                List<String> arguments = call.arguments();
                                boolean success = startInForeground(arguments);
                                result.success(success);
                            } 
                            else if (call.method.equals("startInBackground")) {
                                List<String> arguments = call.arguments();
                                boolean success = startInBackground(arguments, result);
                            }
                            else 
                            {
                                result.notImplemented();
                            }
                        }
                );
    }

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
    }

    private boolean startInForeground(List<String> arguments)
    {
        if(CLAID.isRunning())
        {
            return true;
        }
        Logger.logInfo("Start in foreground called");
        if(arguments.size() != 6)
        {
            Logger.logFatal("Failed to execute startInForeground via MethodChannel function. Expected 6 arguments but got: " + arguments.size());
            return false;
        }
        String socketPath = arguments.get(0);
        String configFilePath = arguments.get(1);
        String hostId = arguments.get(2);
        String userId = arguments.get(3);
        String deviceId = arguments.get(4);
        String specialPermissionsConfigIdentifier = arguments.get(5);

        Logger.logInfo(socketPath + " " + configFilePath + " " + hostId + " " + userId);

        CLAIDSpecialPermissionsConfig specialPermissionsConfig = CLAIDSpecialPermissionsConfig.fromIdentifier(specialPermissionsConfigIdentifier);

        if(specialPermissionsConfig == null)
        {
            Logger.logFatal("Failed to execute startInForeground via MethodChannel function." +
                    "Invalid CLAIDSpecialPermissionsConfigIdentifier \"" + specialPermissionsConfigIdentifier + "\"");
            return false;
        }
        Logger.logInfo("CLAID42 StartInForeground");

        boolean result =  CLAID.startInForeground(getApplicationContext(), socketPath,
                configFilePath, hostId, userId, deviceId, specialPermissionsConfig);
        Logger.logInfo("CLAID42 StartInForeground done");
        return result;

    }

    private boolean startInBackground(List<String> arguments, MethodChannel.Result result)
    {
        Logger.logInfo("Start in background called");
        if(arguments.size() != 7)
        {
            Logger.logFatal("Failed to execute startInBackground via MethodChannel function. Expected 7 arguments but got: " + arguments.size());
            return false;
        }
        String socketPath = arguments.get(0);
        String configFilePath = arguments.get(1);
        String hostId = arguments.get(2);
        String userId = arguments.get(3);
        String deviceId = arguments.get(4);
        String specialPermissionsConfigIdentifier = arguments.get(5);
        String persistanceConfigIdentifier = arguments.get(6);

        CLAIDSpecialPermissionsConfig specialPermissionsConfig = CLAIDSpecialPermissionsConfig.fromIdentifier(specialPermissionsConfigIdentifier);
        CLAIDPersistanceConfig persistanceConfig = CLAIDPersistanceConfig.fromIdentifier(persistanceConfigIdentifier);

        if(specialPermissionsConfig == null)
        {
            Logger.logFatal("Failed to execute startInBackground via MethodChannel function." +
                    "Invalid CLAIDSpecialPermissionsConfigIdentifier \"" + specialPermissionsConfigIdentifier + "\"");
            return false;
        }

        if(persistanceConfig == null)
        {
            Logger.logFatal("Failed to execute startInBackground via MethodChannel function." +
                    "Invalid CLAIDPersistanceConfig \"" + persistanceConfigIdentifier + "\"");
            return false;
        }

        if(CLAID.isRunning())
        {
            result.success(true);
            return true;
        }

        boolean startResult = CLAID.startInBackground(getApplicationContext(), socketPath,
                configFilePath, hostId, userId, deviceId, 
                specialPermissionsConfig, persistanceConfig);

        if(!startResult)
        {
            result.success(false);
            return false;
        }
        backgroundStarted = false; 
        CLAID.onStarted(() -> result.success(true));

        return startResult;
    }
}
