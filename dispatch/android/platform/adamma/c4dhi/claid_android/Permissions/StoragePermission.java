package adamma.c4dhi.claid_android.Permissions;
import static android.content.Intent.FLAG_ACTIVITY_NEW_TASK;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid_platform_impl.CLAID;
import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.Settings;
import java.io.File;
import java.io.FileOutputStream;

public class StoragePermission extends Permission {
    private final static int APP_STORAGE_REQUEST_CODE = 400;
    private static final String userDialogTitle = "You need to allow storage permission";
    private static final String userDialogBody = "In the following screen you need to allow" +
            " storage permission to use this app";
    private static final String[] PERMISSIONS_STORAGE = {
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };


    @Override
    public boolean isGranted() {
        // There is currently no way to request becoming an ExternalStorageManager on WearOS.
        if(CLAID.isWearOS())
        {
            return wearOSHasStoragePermissions();
        }
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R)
        {
            return super.getContext().checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED
                && super.getContext().checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED;
        }
        else
        {
            return Environment.isExternalStorageManager();
        }
    }

    @Override
    public void blockingRequest() {
        if (isGranted())
            System.out.println("We have storage permissions");
        else {
            this.startIntentWithExtras(PERMISSIONS_STORAGE, APP_STORAGE_REQUEST_CODE, userDialogTitle, userDialogBody);
        }
        while (!isGranted()){}
    }

    @Override
    public void startIntentWithExtras(String[] permissions, int requestCode, String dialogTitle, String dialogBody)
    {
        Logger.logInfo("Start intent with extras.");
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.R ||  CLAID.isWearOS())
        {
            super.startIntentWithExtras(permissions, requestCode, dialogTitle, dialogBody);
        }
        else {

            Logger.logInfo("Is WearOS ? " + CLAID.isWearOS());
            if(!CLAID.isWearOS())
            {
                // Manage all files intent not (yet) available on WearOS. 
                // Currently, there is no option available to manage all files on WearOS.
                try {
                    Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
                    intent.addCategory("android.intent.category.DEFAULT");
                    intent.setData(Uri.parse(String.format("package:%s", super.getContext().getPackageName())));
                    super.getContext().startActivity(intent);
                } catch (Exception e) {
                    Intent intent = new Intent();
                    intent.setAction(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
                    intent.setFlags(FLAG_ACTIVITY_NEW_TASK);
                    super.getContext().startActivity(intent);
                }
            }
            

            // This is used to prevent line 76 (super.startIntentWithExtra) to start
            // ... before settings are prompted using Intent, in the above lines
            try {
                Thread.sleep(2000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            Logger.logInfo("Waiting for foreground1");

            while (!super.isAppOnForeground()) {}
            Logger.logInfo("Waiting for foreground2");

            if (!isGranted())
            {
                Logger.logInfo("Waiting for foregorund 3");
                try {
                    Thread.sleep(2000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                startIntentWithExtras(PERMISSIONS_STORAGE,
                        APP_STORAGE_REQUEST_CODE, userDialogTitle, userDialogBody);
            }            
        }
    }

    
    public boolean wearOSHasStoragePermissions()
    {

        String filepath = CLAID.getMediaDirPath(CLAID.getContext()) + "/claid_test_storage_permissions.txt";
        FileOutputStream fos = null;
        try {
            fos = new FileOutputStream(filepath);
            byte[] buffer = "test".getBytes();
            fos.write(buffer, 0, buffer.length);
            fos.close();
            return true;
        } catch (Exception e)
        {
            return false;
        }
    }

}
