package adamma.c4dhi.claid_android.CLAIDServices;

import android.content.Context;
import android.content.Intent;

import adamma.c4dhi.claid_android.CLAIDServices.MaximumPermissionsPerpetualService;
import adamma.c4dhi.claid_android.Permissions.Permission;
import adamma.c4dhi.claid.Logger.Logger;


public class ServiceManager
{
    public static void startMaximumPermissionsPerpetualService(Context context, 
        final String socketPath, final String configFilePath, final String hostId, final String userId, final String deviceId)
    {   
        Permission.setContext(context);
        // Request all permissions.
        if(!MaximumPermissionsPerpetualService.requestRequiredPermissions())
        {
            Logger.logError("Failed to start CLAIDServices.MaximumPermissionsPerpetualService.\n" + 
                            "Required permissions were not granted.");
        }


        Intent serviceIntent = new Intent(context, MaximumPermissionsPerpetualService.class);
        serviceIntent.putExtra("socketPath", socketPath);
        serviceIntent.putExtra("configFilePath", configFilePath);
        serviceIntent.putExtra("hostId", hostId);
        serviceIntent.putExtra("userId", userId);
        serviceIntent.putExtra("deviceId", deviceId);


        context.startForegroundService(serviceIntent);
    }

}

