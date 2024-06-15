package adamma.c4dhi.claid_android.Package;

import adamma.c4dhi.claid.Logger.Logger;

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
        System.loadLibrary(name);
        return true;
    }
}