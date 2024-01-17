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

public class ModuleFactory
{
    private Map<String, Class<? extends Module>> registeredModuleClasses = new HashMap<>();


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
        if (!registeredModuleClasses.containsKey(className)) 
        {
            return false;
        } 
        else 
        {
            Class<?> classType = registeredModuleClasses.get(className);
            String annotateModuleFunction = "annotateModule";


            Class<?>[] parameterTypes = {ModuleAnnotator.class};

            try 
            {
                // Attempt to get the method with the specified name and parameter types
                Method method = classType.getDeclaredMethod(annotateModuleFunction, parameterTypes);

                Object result = method.invoke(null, moduleAnnotator);
                return true;
            } 
            catch (NoSuchMethodException e) 
            {
               // Logger.logError("Method not found: " + annotateModuleFunction);
                return false;
            } 
            catch (IllegalAccessException | InvocationTargetException e) 
            {
              //  Logger.logError("Error invoking method: " + e.getMessage());
                return false;
            }
        }
    }



    
}