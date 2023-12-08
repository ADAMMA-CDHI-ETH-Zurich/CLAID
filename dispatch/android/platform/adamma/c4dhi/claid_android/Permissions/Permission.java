package adamma.c4dhi.claid_android.Permissions;

import static adamma.c4dhi.claid_android.Permissions.PermissionActivity.EXTRA_DIALOG_BODY;
import static adamma.c4dhi.claid_android.Permissions.PermissionActivity.EXTRA_DIALOG_TITLE;
import static adamma.c4dhi.claid_android.Permissions.PermissionActivity.EXTRA_PERMISSIONS;
import static adamma.c4dhi.claid_android.Permissions.PermissionActivity.EXTRA_REQUEST_CODE;

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import java.util.List;

import adamma.c4dhi.claid_platform_impl.CLAID;
import adamma.c4dhi.claid.Logger.Logger;
public abstract class Permission
{
    public abstract void blockingRequest();

    public abstract boolean isGranted();

    public void startIntentWithExtras(String[] permissions, int requestCode, String dialogTitle, String dialogBody)
    {
        Logger.logInfo("request microphone permission4");

        while (!isAppOnForeground()){}
        Logger.logInfo("request microphone permission5");

        Intent intent = new Intent(getContext(), PermissionActivity.class);
        intent.putExtra(EXTRA_PERMISSIONS, permissions);
        intent.putExtra(EXTRA_REQUEST_CODE, requestCode);
        intent.putExtra(EXTRA_DIALOG_TITLE, dialogTitle);
        intent.putExtra(EXTRA_DIALOG_BODY, dialogBody);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        Logger.logInfo("request microphone permission6");

        getContext().startActivity(intent);
        Logger.logInfo("request microphone permission7");

    }

    public boolean isAppOnForeground() {
        ActivityManager activityManager = (ActivityManager) this.getContext().getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> appProcesses = activityManager.getRunningAppProcesses();
        if (appProcesses == null) {
            return false;
        }
        final String packageName = this.getContext().getPackageName();
        for (ActivityManager.RunningAppProcessInfo appProcess : appProcesses) {
            if (appProcess.importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_FOREGROUND && appProcess.processName.equals(packageName)) {
                return true;
            }
        }
        return false;
    }

    private static Context context = null;

    public static void setContext(Context context)
    {
        Permission.context = context;
    }

    public Context getContext() 
    {
        if(Permission.context != null)
        {
            return Permission.context;

        }
        return (Context) CLAID.getContext();
    }

}
