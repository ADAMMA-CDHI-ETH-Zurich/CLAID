package adamma.c4dhi.claid;

public class CLAID
{
    static {
        System.loadLibrary("claid_capi");
    }

    public static native void start(final String socketPath, 
        final String configFilePath, final String hostId, final String userId, final String deviceId);
}