package adamma.c4dhi.claid_android.Configuration;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
// A configuration for CLAID services. Specifies the strategies the
// CLAID service will follow in order to keep running as long as possible.
// For best endurance, use the maximumEnduranceConfig().

public class CLAIDPersistanceConfig 
{
    // Service will start when the device boots.
    public boolean RESTART_ON_BOOT = false;

    // If enabled, a separate worker will periodically monitor whether the CLAID service is running every 16 minutes (fastest period supported by Android).
    // If the server was stopped (e.g., due to a crash), it will be restarted.
    // Requires to disable battery optimizations for the App since Android 10.
    public boolean MONITOR_TRY_RESTART_IF_CRASHED_OR_EXITED = false;

    public boolean DISABLE_BATTERY_OPTIMIZATIONS = false;


    
    // A configuration that tells CLAID to try everything that's possible
    // to keep the service running as long as possible. 
    // Might require additional permissions that will be requested by CLAID automatically.
    public static CLAIDPersistanceConfig maximumPersistance()
    {
        CLAIDPersistanceConfig config = new CLAIDPersistanceConfig();
        config.RESTART_ON_BOOT = true;
        config.MONITOR_TRY_RESTART_IF_CRASHED_OR_EXITED = true;
        config.DISABLE_BATTERY_OPTIMIZATIONS = true;
        return config;
    }


    // A configration that tells CLAID to simply let go when the service is terminated.
    // No effort will be taken to restart the service.
    public static CLAIDPersistanceConfig minimumPersistance()
    {
        CLAIDPersistanceConfig config = new CLAIDPersistanceConfig();
        config.RESTART_ON_BOOT = false;
        config.MONITOR_TRY_RESTART_IF_CRASHED_OR_EXITED = false;
        config.DISABLE_BATTERY_OPTIMIZATIONS = false;
        return config;
    }

    public static boolean writeToFile(final String path, CLAIDPersistanceConfig config)
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

    public static CLAIDPersistanceConfig readFromFile(final String path)
    {
        CLAIDPersistanceConfig deserialized;
        try {
            FileInputStream fileIn = new FileInputStream(path);
            ObjectInputStream in = new ObjectInputStream(fileIn);
            deserialized = (CLAIDPersistanceConfig) in.readObject();
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
