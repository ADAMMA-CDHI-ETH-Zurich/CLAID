package adamma.c4dhi.claid_android.Package;

import android.content.Context;



import java.io.IOException;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.List;
import java.util.Set;
import java.util.Map;
import java.util.HashMap;

import adamma.c4dhi.claid.Logger.Logger;
import dalvik.system.DexFile;
import dalvik.system.PathClassLoader;

public class CLAIDPackageLoader 
{
    static private List<String> supportedPackages = Arrays.asList(new String[]{"adamma.", ".claid."});
    static private Map<String, CLAIDPackage> loadedPackages = new HashMap<>();
    static private List<Class<? extends CLAIDPackage>> scannedPackages;

    static public void loadPackage(CLAIDPackage claidPackage)
    {
        String packageName = claidPackage.getClass().getName();
        
        if(loadedPackages.containsKey(packageName))
        {
            Logger.logWarning("Not loading CLAID package \"" + packageName + "\", because it is already loaded");
            return;
        }
        Logger.logInfo("Loading package \"" + packageName + "\"");
        claidPackage.register();
        loadedPackages.put(packageName, claidPackage);
    }

    static public void loadPackages(Context context)
    {
        try {
            scannedPackages = scanPackages(context);
        } catch (Exception e) {
            Logger.logFatal(e.getMessage());
            return;
        } 

        Logger.logInfo("Found " + scannedPackages.size() + " CLAID packages");
        for(Class<? extends CLAIDPackage> packageClass : scannedPackages)
        {
            Logger.logInfo("Loading CLAID package " +  packageClass.getName());
            try {
                CLAIDPackage claidPackage = packageClass.getDeclaredConstructor().newInstance();
                CLAIDPackageLoader.loadPackage(claidPackage);
            } catch (Exception e) {
                Logger.logFatal(e.getMessage());
                return;
            }
        }

    }

   static private List<Class<? extends CLAIDPackage>> scanPackages(Context context) throws IOException, ClassNotFoundException, NoSuchMethodException
   {
        System.out.println("Scanning classes");
        List<Class<? extends CLAIDPackage>> foundPackages = new ArrayList<>();
        PathClassLoader classLoader = (PathClassLoader) context.getClassLoader();
        //PathClassLoader classLoader = (PathClassLoader) Thread.currentThread().getContextClassLoader();//This also works good
        DexFile dexFile = new DexFile(context.getPackageCodePath());
        Enumeration<String> classNames = dexFile.entries();
        while (classNames.hasMoreElements()) {

            String className = classNames.nextElement();

            boolean validPackage = false;
            for(String packageIdentifier : supportedPackages)
            {
                if(className.contains(packageIdentifier))
                {
                    validPackage = true;
                    break;
                }
            }
            if(!validPackage)
            {
                continue;
            }

            Class<?> clz = classLoader.loadClass(className);
            if(clz.isAnnotationPresent(CLAIDPackageAnnotation.class))
            {
                System.out.println("Found class " + className);
                foundPackages.add((Class<? extends CLAIDPackage>) clz);
                Logger.logInfo("Found packages size: " + foundPackages);

            }
        }
        Logger.logInfo("Found packages size: " + foundPackages);
        return foundPackages;
   }
}
