package adamma.c4dhi.claid.Module;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;


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
}