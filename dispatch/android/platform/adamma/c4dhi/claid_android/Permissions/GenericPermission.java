package adamma.c4dhi.claid_android.Permissions;
import android.content.pm.PackageManager;


// E.g. usage:
// GenericPermission genericPermission = new GenericPermission(new String[]{"android.permission.CAMERA"});
// genericPermission.blockingRequest();
public class GenericPermission extends Permission {
    private static final int GENERIC_REQUEST_CODE = 100;
    private String[] permissionNames;
    private static final String userDialogTitle = "You need to allow permissions";
    private String userDialogBody = "In the following screen you need to allow %s\n" +
            "to use this app. If you can't see the option you need to open Settings->" +
            "Apps->CLAIDDemo->Permissions.";

    public GenericPermission(String[] manifestPermissionNames) {
        this.permissionNames = manifestPermissionNames;
        userDialogBody = String.format(userDialogBody,
                String.join(" and ", manifestPermissionNames));
    }

    @Override
    public boolean isGranted() {
        boolean isGranted = true;
        for (String singlePermission : permissionNames) {
            isGranted = (super.getContext().checkSelfPermission(singlePermission)
                    == PackageManager.PERMISSION_GRANTED) && isGranted;
        }
        return isGranted;
    }

    @Override
    public void blockingRequest() {
        if (!isGranted()) {
            super.startIntentWithExtras(permissionNames, GENERIC_REQUEST_CODE, userDialogTitle, userDialogBody);
            while (!isGranted()) {};
        }
    }
}
