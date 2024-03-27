package adamma.c4dhi.claid_android.collectors.battery;

import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.ChannelData;
import adamma.c4dhi.claid.Module.Module;
import adamma.c4dhi.claid.Module.Properties;

import adamma.c4dhi.claid_platform_impl.CLAID;

import adamma.c4dhi.claid_sensor_data.BatterySample;
import adamma.c4dhi.claid_sensor_data.BatteryData;
import adamma.c4dhi.claid_sensor_data.BatteryState;


import static android.app.AlarmManager.RTC_WAKEUP;
import static android.app.PendingIntent.FLAG_UPDATE_CURRENT;
import static android.app.PendingIntent.FLAG_MUTABLE;

import static android.app.PendingIntent.getBroadcast;
import static android.content.Context.ALARM_SERVICE;
import static android.app.AlarmManager.RTC_WAKEUP;
import static android.app.PendingIntent.FLAG_UPDATE_CURRENT;
import static android.app.PendingIntent.getBroadcast;
import static android.content.Context.ALARM_SERVICE;


import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;


import java.util.Calendar;

public class AlarmBatteryCollector extends Module
{

    public void initialize(Properties properties)
    {
        moduleInfo("AlarmBatteryCollector init");
        Context context = CLAID.getContext();

        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(System.currentTimeMillis()+ 10000);
        //noinspection ConstantConditions


      //  startActivity(new Intent(android.provider.Settings.ACTION_REQUEST_SCHEDULE_EXACT_ALARM, Uri.parse("package:"+ getPackageName())));
        final AlarmManager am = (AlarmManager) context.getSystemService(ALARM_SERVICE);
        final Intent intent = new Intent(context, adamma.c4dhi.claid_android.collectors.battery.BatteryReceiver.class);
        final PendingIntent pending = getBroadcast(context, 0, intent, FLAG_UPDATE_CURRENT|FLAG_MUTABLE);


        am.setExactAndAllowWhileIdle(RTC_WAKEUP, calendar.getTimeInMillis(), pending);

    }

   

}
