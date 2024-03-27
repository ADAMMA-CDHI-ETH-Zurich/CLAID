package adamma.c4dhi.claid.demo_app;

import static android.app.AlarmManager.RTC_WAKEUP;
import static android.app.PendingIntent.FLAG_MUTABLE;
import static android.app.PendingIntent.FLAG_UPDATE_CURRENT;
import static android.app.PendingIntent.getBroadcast;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.view.WindowManager;

import java.util.Calendar;

import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid_android.Configuration.CLAIDPersistanceConfig;
import adamma.c4dhi.claid_android.Configuration.CLAIDSpecialPermissionsConfig;
import adamma.c4dhi.claid_platform_impl.CLAID;
import adamma.c4dhi.claid_platform_impl.PersistentModuleFactory;

public class MainActivity extends Activity {
    PersistentModuleFactory factory;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(System.currentTimeMillis()+ 5000);
        //noinspection ConstantConditions




        factory = ((MyApplication) getApplication()).factory;
        factory = (PersistentModuleFactory) CLAID.registerDefaultModulesToFactory((ModuleFactory) factory);

        CLAID.startInPersistentService(getApplicationContext(), "assets://BatteryManagerTest.json",
                "Smartphone",
                "device",
                "user",
                factory,
                CLAIDSpecialPermissionsConfig.regularConfig(),
                CLAIDPersistanceConfig.maximumPersistance());

   //     CLAID.enableKeepAppAwake(CLAID.getContext());

    }
}
