/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer, Francesco Feher
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

package adamma.c4dhi.claid_android.Permissions;
import android.content.ComponentName;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.os.Bundle;
import android.view.WindowManager;
import android.app.Activity;
import android.app.AlertDialog;
import adamma.c4dhi.claid.Logger.Logger;

import java.util.Arrays;
import java.util.Objects;
import java.util.concurrent.atomic.AtomicBoolean;
import androidx.core.content.IntentCompat;
import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import java.util.List;
// "Invisible" activity used to ask permissions
public class PermissionActivity extends Activity {
    public static final String EXTRA_PERMISSIONS = "permissions";
    public static final String EXTRA_REQUEST_CODE = "requestCode";
    public static final String EXTRA_DIALOG_TITLE = "dialogTitle";
    public static final String EXTRA_DIALOG_BODY = "dialogBody";
    public static final String EXTRA_REQUIRES_PERMISSION_PAGE = "permissionPage";
    public static final String EXTRA_ALWAYS_SHOW_INFO_DIALOG = "alwaysInfoDialog";

    static final int DEFAULT_PERMISSION_REQUEST_CODE = 500;

    private boolean shouldRecheckPermissionsAfterReturningFromPermissionsPage = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        this.setTheme();

        if(getIntent().getBooleanExtra(EXTRA_ALWAYS_SHOW_INFO_DIALOG, false))
        {
            // Should be blocking.
            Logger.logInfo("Always show info dialog is true");
            displayAlertDialog(
                getIntent().getStringExtra(EXTRA_DIALOG_TITLE),
                getIntent().getStringExtra(EXTRA_DIALOG_BODY),
                () -> requestPermissionsFromExtra()
            );
        }
        else
        {
            requestPermissionsFromExtra();
        }

        
    }

    @Override
    protected void onResume()
    {
        super.onResume();
        if(shouldRecheckPermissionsAfterReturningFromPermissionsPage)
        {
            shouldRecheckPermissionsAfterReturningFromPermissionsPage = false;
            recheckPermissionAfterPermissionsPage();
        }
    }

    void requestPermissionsFromExtra()
    {
        String[] permissions = getIntent().getStringArrayExtra(EXTRA_PERMISSIONS);

        int requestCode = getIntent().getIntExtra(EXTRA_REQUEST_CODE, DEFAULT_PERMISSION_REQUEST_CODE);
        this.requestPermissions(permissions, requestCode);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        Logger.logInfo("showing alert -1");
        if (!Arrays.stream(grantResults).allMatch(result -> result == PackageManager.PERMISSION_GRANTED))
        {
            Logger.logInfo("showing alert -2");

            if(!getIntent().getBooleanExtra(EXTRA_REQUIRES_PERMISSION_PAGE, false))
            {
                Logger.logInfo("showing alert -3");

                displayAlertDialogAndRestart(getIntent().getStringExtra(EXTRA_DIALOG_TITLE), getIntent().getStringExtra(EXTRA_DIALOG_BODY));
            }
            else
            {
                displayAlertDialogSendUserToPermissionsPage(getIntent().getStringExtra(EXTRA_DIALOG_TITLE), getIntent().getStringExtra(EXTRA_DIALOG_BODY));
            }
        }
        else
        {
            Logger.logInfo("showing alert -4");

            finish();
        }
    }

    // This function will be called always after the user returns from the permissions page.
    void recheckPermissionAfterPermissionsPage()
    {
        String[] requiredPermissions = getIntent().getStringArrayExtra(EXTRA_PERMISSIONS);

        if(!Arrays.stream(requiredPermissions).allMatch(
            permission -> checkSelfPermission(permission) == PackageManager.PERMISSION_GRANTED))
        {
            displayAlertDialogSendUserToPermissionsPage(
                getIntent().getStringExtra(EXTRA_DIALOG_TITLE), 
                getIntent().getStringExtra(EXTRA_DIALOG_BODY)
            );
        }
        else
        {
            finish();
        }
    }

    private void displayAlertDialog(String dialogTitle, String dialogBody)
    {
        displayAlertDialog(dialogTitle, dialogBody, null);
    }

    private void displayAlertDialog(String dialogTitle, String dialogBody, Runnable runnable)
    {
        new AlertDialog.Builder(this)
        .setTitle(dialogTitle)
        .setMessage(dialogBody)
        .setPositiveButton("OK", (dialog, id) -> {
            if(runnable != null)
            {
                runnable.run();
            }
        })
        .setCancelable(false)
        .setIcon(android.R.drawable.ic_dialog_alert)
        .show();
    }

    private void displayAlertDialogAndRestart(String dialogTitle, String dialogBody)
    {
        displayAlertDialog(dialogTitle, dialogBody, () -> restartApp());
    }


    private void displayAlertDialogSendUserToPermissionsPage(String dialogTitle, String dialogBody)
    {
        displayAlertDialog(dialogTitle, dialogBody, () -> sendUserToPermissionsPageAndRecheckPermissions());
    }

    private void setTheme()
    {
        getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            getWindow().setDecorFitsSystemWindows(false);
        }
        getWindow().setWindowAnimations(0);
        getWindow().setDimAmount(0.0f);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
                WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);
    }

    private void restartApp()
    {
        PackageManager packageManager = this.getPackageManager();
        Intent intent = packageManager.getLaunchIntentForPackage(this
                .getPackageName());
        ComponentName componentName = intent.getComponent();
        Intent mainIntent = Intent.makeRestartActivityTask(componentName);
        this.startActivity(mainIntent);
        Runtime.getRuntime().exit(0);
    }

    
    private void sendUserToPermissionsPage()
    {
        Intent intent = IntentCompat.createManageUnusedAppRestrictionsIntent
                                (getApplicationContext(), getPackageName());
                                
        // Must use startActivityForResult(), not startActivity(), even if
        // you don't use the result code returned in onActivityResult().
        startActivityForResult(intent, 1337);
    }

    private void sendUserToPermissionsPageAndRecheckPermissions()
    {
        sendUserToPermissionsPage();
        // If we would check "isAppOnForeground" already, it would likely be true,
        // as the intent to send the user to the permissions page would not have been executed yet.
        // Thus, sleep a bit before we check isAppOnForeground.
        shouldRecheckPermissionsAfterReturningFromPermissionsPage = true;
    }

    public boolean isAppOnForeground() {
        ActivityManager activityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> appProcesses = activityManager.getRunningAppProcesses();
        if (appProcesses == null) {
            Logger.logInfo("App processes null");
            return false;
        }
        final String packageName = getPackageName();
        for (ActivityManager.RunningAppProcessInfo appProcess : appProcesses) {
            if (appProcess.importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_FOREGROUND && appProcess.processName.equals(packageName)) {
                Logger.logInfo("importance ");

                return true;
            }
            else {
                Logger.logInfo("importance fail");

            }
        }
        Logger.logInfo("end of function");

        return false;
    }

    protected void sleep(int milliseconds)
    {
        try{
            Thread.sleep(milliseconds);
        }
        catch(InterruptedException e)
        {
            
        }
    }
}