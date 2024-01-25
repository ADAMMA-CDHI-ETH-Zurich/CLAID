package adamma.c4dhi.claid_android.Receivers;

import android.app.admin.DeviceAdminReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.app.admin.DevicePolicyManager;

public class DeviceOwnerReceiver extends DeviceAdminReceiver {

    @Override
    public void onProfileProvisioningComplete(Context context, Intent intent) {
        DevicePolicyManager manager = (DevicePolicyManager) context.getSystemService(Context.DEVICE_POLICY_SERVICE);
        ComponentName componentName = new ComponentName(context.getApplicationContext(), DeviceOwnerReceiver.class);

        manager.setProfileName(componentName, "CLAID");
    }
}
