package adamma.c4dhi.claid.Module;

import adamma.c4dhi.claid.Module.ModuleAnnotator;
import adamma.c4dhi.claid.ModuleAnnotation;
import adamma.c4dhi.claid.Logger.Logger;


import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

import adamma.c4dhi.claid_platform_impl.CLAID;

public class ModuleFactory
{
    private Map<String, Class<? extends Module>> registeredModuleClasses = new HashMap<>();

    public ModuleFactory()
    {
        CLAID.registerDefaultModulesToFactory(this);
    }

    public boolean registerModule(Class<? extends Module> clz)
    {
        String className = clz.getSimpleName();
        this.registeredModuleClasses.put(className, clz);

        
        System.out.println("Register class " + className);
        return true;
    }

    public Module getInstance(String className, String moduleId)
    {
        if(this.registeredModuleClasses.containsKey(className))
        {
            try
            {
                Module module = this.registeredModuleClasses.get(className).getDeclaredConstructor().newInstance();
                module.setId(moduleId);
                return module;
            }
            catch(NoSuchMethodException e)
            {
                e.printStackTrace();
                return null;
            }
            catch(InstantiationException e)
            {
                e.printStackTrace();
                return null;
            }
            catch(IllegalAccessException e)
            {
                e.printStackTrace();
                return null;
            }
            catch(InvocationTargetException e)
            {
                e.printStackTrace();
                return null;
            }
        }
        return null;
    }

    public boolean isModuleClassRegistered(String moduleClass)
    {
        return this.registeredModuleClasses.containsKey(moduleClass);
    }

    public ArrayList<String> getRegisteredModuleClasses()
    {
        return new ArrayList<String>(this.registeredModuleClasses.keySet());
    }

    public boolean getModuleAnnotation(String className, ModuleAnnotator moduleAnnotator) 
    {
        Logger.logInfo("Getting module annotations for " + className + " 1");
        if (!registeredModuleClasses.containsKey(className)) 
        {
            Logger.logInfo("Getting module annotations for " + className + " 2");

            return false;
        } 
        else 
        {
            Logger.logInfo("Getting module annotations for " + className + "3");

            Class<?> classType = registeredModuleClasses.get(className);
            String annotateModuleFunction = "annotateModule";


            Class<?>[] parameterTypes = {ModuleAnnotator.class};

            try 
            {
                Logger.logInfo("Getting module annotations for " + className + " 4");

                // Attempt to get the method with the specified name and parameter types
                Method method = classType.getDeclaredMethod(annotateModuleFunction, parameterTypes);
                Logger.logInfo("Getting module annotations for " + className + " 5");
                Object result = method.invoke(null, moduleAnnotator);
                return true;
            } 
            catch (NoSuchMethodException e) 
            {
                Logger.logInfo("Getting module annotations for " + className + " 6 " + e.getMessage());

               // Logger.logError("Method not found: " + annotateModuleFunction);
                return false;
            } 
            catch (IllegalAccessException e) 
            {
                Logger.logInfo("Getting module annotations for " + className + " 7 " + e.getMessage() + " " + e + " " + e.getCause());
                e.printStackTrace();

              //  Logger.logError("Error invoking method: " + e.getMessage());
                return false;
            }
            catch(InvocationTargetException e)
            {
                Logger.logInfo("Getting module annotations for " + className + " 8 " + e.getMessage() + " " + e + " " + e.getCause());
                e.printStackTrace();

              //  Logger.logError("Error invoking method: " + e.getMessage());
                return false;
            }
        }
    }



    
}