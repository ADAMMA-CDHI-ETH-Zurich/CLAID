package adamma.c4dhi.claid_android.Package;

import android.content.Context;
import android.content.res.AssetManager;


import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
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
        List<String> packageClassNames = scanPackages(context);

        

        Logger.logInfo("Found " + packageClassNames.size() + " CLAID packages");
        for(String packageClassName : packageClassNames)
        {
            Logger.logInfo("Loading CLAID package " +  packageClassName);
            Class<CLAIDPackage> packageClass;
            
            try
            {
                packageClass = (Class<CLAIDPackage>) Class.forName(packageClassName);
            }
            catch(ClassNotFoundException e)
            {
                Logger.logFatal("Found invalid CLAID package \"" + packageClassName + "\". Package found in claidpackage file but not in the Classpath");
                return;
            }
            if(!packageClass.isAnnotationPresent(CLAIDPackageAnnotation.class))
            {
                Logger.logFatal("Found invalid CLAID package \"" + packageClassName + "\". Package does not contain CLAIDPackageAnnotation.");
                return;
            }

            try 
            {
                CLAIDPackage claidPackage = packageClass.getDeclaredConstructor().newInstance();
                CLAIDPackageLoader.loadPackage(claidPackage);
            } 
            catch (Exception e) 
            {
                Logger.logFatal(e.getMessage());
                return;
            }
        }

    }

    public static List<String> scanPackages(Context context) 
    {
        List<String> packageNames = new ArrayList<>();

        AssetManager assetManager = context.getAssets();
        try {
            // List all files in the assets folder
            String[] files = assetManager.list("");
            if (files != null) 
            {
                for (String filename : files) 
                {
                    // Check if the file is a .claidpackage file
                    if (filename.endsWith(".claidpackage")) 
                    {
                        Logger.logInfo("Found CLAID package file " + filename);
                        // Read lines from the claidpackage file
                        BufferedReader reader = new BufferedReader(
                                new InputStreamReader(assetManager.open(filename)));

                        String line;
                        while ((line = reader.readLine()) != null) 
                        {
                            // Each line corresponds to a package name
                            if (!line.isEmpty()) 
                            {
                                Logger.logInfo("Found CLAID package class " + line.trim());
                                packageNames.add(line.trim());
                            }
                        }

                        // Close the reader
                        reader.close();
                    }
                }
            }
        } 
        catch (IOException e) 
        {
            e.printStackTrace();
        }

        return packageNames;
    }

//    static private List<Class<? extends CLAIDPackage>> scanPackages(Context context) throws IOException, ClassNotFoundException, NoSuchMethodException
//    {
//         System.out.println("Scanning classes");
//         List<Class<? extends CLAIDPackage>> foundPackages = new ArrayList<>();
//         PathClassLoader classLoader = (PathClassLoader) context.getClassLoader();
//         //PathClassLoader classLoader = (PathClassLoader) Thread.currentThread().getContextClassLoader();//This also works good
//         DexFile dexFile = new DexFile(context.getPackageCodePath());
//         Enumeration<String> classNames = dexFile.entries();
//         while (classNames.hasMoreElements()) {

//             String className = classNames.nextElement();

//             boolean validPackage = false;
//             for(String packageIdentifier : supportedPackages)
//             {
//                 if(className.contains(packageIdentifier))
//                 {
//                     validPackage = true;
//                     break;
//                 }
//             }
//             if(!validPackage)
//             {
//                 continue;
//             }

//             Class<?> clz = classLoader.loadClass(className);
//             if(clz.isAnnotationPresent(CLAIDPackageAnnotation.class))
//             {
//                 System.out.println("Found class " + className);
//                 foundPackages.add((Class<? extends CLAIDPackage>) clz);
//                 Logger.logInfo("Found packages size: " + foundPackages);

//             }
//         }
//         Logger.logInfo("Found packages size: " + foundPackages);
//         return foundPackages;
//    }
}
