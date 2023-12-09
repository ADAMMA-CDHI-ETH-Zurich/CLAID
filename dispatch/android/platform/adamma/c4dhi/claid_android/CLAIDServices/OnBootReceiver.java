package adamma.c4dhi.claid_android.CLAIDServices;
import adamma.c4dhi.claid_android.CLAIDServices.ServiceManager;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Debug;
import android.util.Log;

import java.io.FileOutputStream;
import java.security.Provider.Service;
import java.text.SimpleDateFormat;
import java.util.Date;

import adamma.c4dhi.claid.Logger.Logger;

public class OnBootReceiver extends BroadcastReceiver
{
    @Override
    public void onReceive(Context context, Intent intent)
    {
        
        try {
            
            Logger.logInfo("onBootReceiver triggered");
            String action = intent.getAction();

            if(action != null)
            {
                if (action.equals(Intent.ACTION_BOOT_COMPLETED) )
                {
                    Logger.logInfo("onBootReceiver is ACTION_BOOT_COMPLETED");
                    if(ServiceManager.shouldStartMaximumPermissionsPerpetualServiceOnBoot(context))
                    {
                        Logger.logInfo("onBootReceiver shouldStartMaximumPermissionsPerpetualServiceOnBoot is true, trying to start Service.");
                        ServiceManager.executeConfiguredStartupMethodForMaximumPermissionsPerpetualService(context);
                    }
                }
            }
        } 
        catch (Exception e)
        {
            Logger.logFatal(e.getMessage());
        }
        Logger.logInfo("CLAID onBoot !");

    }
}