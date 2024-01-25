package adamma.c4dhi.claid_android.Configuration;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
// A configuration specifying the power ("mightiness")
// CLAID has to perform "above standard" functions (i.e., 
// functions that are typically not available for regular Android apps, such as disabling or enabling Wifi from the background without user intervention).

// In most cases, CLAIDMightinessConfig.regularConfig() is sufficient and the CLAID App will have as much power as any regular Android app.
// For some cases, you might require more Power, e.g., to manage the external storage, start activities from the background, disable or enable wifi without user intervention.


public class CLAIDMightinessConfig 
{
    // Allows the CLAID app to manage all files on the internal storage or external sd card.
    public boolean MANAGE_ALL_STORAGE = false;

    // Allows to restart activities from the background and makes Android more reluctant to kill the CLAID service.
    public boolean DISABLE_BATTERY_OPTIMIZATIONS = false;

    // Device Owner Apps can do a lot more than regular Apps on modern Android OS versions (or, can do what regular Apps were allowed to do in old Android versions anyways).
    // Functions such as disabling or enabling wifi or bluetooth from the background, without the user having to intervene,
    // are available for device owner apps. Device owner Apps can take major control over the OS.
    public boolean BE_DEVICE_OWNER = false;

    public static CLAIDMightinessConfig almightyCLAID()
    {
        CLAIDMightinessConfig config = new CLAIDMightinessConfig();
        config.MANAGE_ALL_STORAGE = true;
        config.DISABLE_BATTERY_OPTIMIZATIONS = true;
        config.BE_DEVICE_OWNER = true;

        return config;
    }
    
    public static CLAIDMightinessConfig regularConfig()
    {
        CLAIDMightinessConfig config = new CLAIDMightinessConfig();
        config.MANAGE_ALL_STORAGE = false;
        config.DISABLE_BATTERY_OPTIMIZATIONS = false;
        config.BE_DEVICE_OWNER = false;

        return config;
    }

    public static CLAIDMightinessConfig allStorageAccessConfig()
    {
        CLAIDMightinessConfig config = new CLAIDMightinessConfig();

        config.MANAGE_ALL_STORAGE = true;
        config.DISABLE_BATTERY_OPTIMIZATIONS = false;
        config.BE_DEVICE_OWNER = false;

        return config;
    }

    public static boolean writeToFile(final String path, CLAIDMightinessConfig config)
    {
        try {
            FileOutputStream fileOut = new FileOutputStream(path);
            ObjectOutputStream out = new ObjectOutputStream(fileOut);
            out.writeObject(config);
            out.close();
            fileOut.close();
        } catch (IOException i) {
            i.printStackTrace();
            return false;
        }
        return true;
    }

    public CLAIDMightinessConfig readFromFile(final String path)
    {
        CLAIDMightinessConfig deserialized;
        try {
            FileInputStream fileIn = new FileInputStream(path);
            ObjectInputStream in = new ObjectInputStream(fileIn);
            deserialized = (CLAIDMightinessConfig) in.readObject();
            in.close();
            fileIn.close();
            return deserialized;
        } 
        catch (IOException i) 
        {
            i.printStackTrace();
            return null;
        } 
        catch (ClassNotFoundException c) 
        {
            c.printStackTrace();
            return null;
        }
    }


}
