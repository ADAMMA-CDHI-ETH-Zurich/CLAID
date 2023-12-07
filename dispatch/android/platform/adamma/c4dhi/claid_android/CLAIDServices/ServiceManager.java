package adamma.c4dhi.claid_android.CLAIDSevices;

import android.content.Context;
import android.content.Intent;

import androidx.core.content.ContextCompat;


public class ServiceManager
{
    public static void startMaximumPermissionsPerpetualService(Context context, final String configPath)
    {
        Intent serviceIntent = new Intent(context, MaximumPermissionsPerpetualService.class);
        serviceIntent.putExtra("CLAIDConfigPath", configPath);

        ContextCompat.startForegroundService(context, serviceIntent);
    }

}

public class ServiceManager {

    
    
}
