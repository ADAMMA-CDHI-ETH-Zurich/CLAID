package adamma.c4dhi.claid_android.Package;

import android.content.Context;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid_platform_impl.CLAID;

public abstract class CLAIDPackage
{
    private boolean nativeComponentLoaded = false;

    public abstract void register();

    protected boolean loadNativeComponent(String name)
    {
        if(nativeComponentLoaded)
        {
            Logger.logError("Failed to load native library \"" + name + "\". Library was already loaded.");
            return false;
        }
        Context context = CLAID.getContext();
        String path = context.getApplicationInfo().nativeLibraryDir + "/" + name;
        long handle = CLAID.nativeLoadSharedLibGlobal(path);

        if(handle == 0)
        {
            Logger.logFatal("Failed to load native plugin \"" + name + "\". Make sure You have android:extractNativeLibs=\"true\" in your AndroidManifest.xml.");
        }
        return true;
    }
}