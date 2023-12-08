package adamma.c4dhi.claid_android.Permissions;
import android.Manifest;
import android.content.pm.PackageManager;
import adamma.c4dhi.claid.Logger.Logger;


public class MicrophonePermission extends Permission {
    private static final int MICROPHONE_REQUEST_CODE = 500;
    private static final String userDialogTitle = "You need to allow microphone permission";
    private static final String userDialogBody = "In the following screen you need to allow " +
            "microphone permissions to use this app. If you can't see the option you need " +
            "to open\n" + "Settings->Apps->CLAIDDemo->Permissions.";
    private static final String[] RECORDING_PERMISSION = {
            Manifest.permission.RECORD_AUDIO
    };


    @Override
    public boolean isGranted() {
        return (super.getContext().checkSelfPermission(RECORDING_PERMISSION[0])
                == PackageManager.PERMISSION_GRANTED);
    }

    @Override
    public void blockingRequest() {
        Logger.logInfo("request microphone permission3");

        if (!isGranted()) {
            Logger.logInfo("Looping1");

            super.startIntentWithExtras(RECORDING_PERMISSION, MICROPHONE_REQUEST_CODE, userDialogTitle, userDialogBody);
            Logger.logInfo("Looping2");

            try{
                Thread.sleep(200);
            }
            catch(InterruptedException e)
            {

            }
        }
        while (!isGranted()) {}
        Logger.logInfo("granted");

    }
}
