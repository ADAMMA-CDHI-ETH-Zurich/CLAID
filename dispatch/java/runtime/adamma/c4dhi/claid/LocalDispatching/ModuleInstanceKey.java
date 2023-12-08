package adamma.c4dhi.claid.LocalDispatching;

public class ModuleInstanceKey {
    public String moduleId;
    public String moduleClass;
    
    public ModuleInstanceKey()
    {
        this.moduleId = "";
        this.moduleClass = "";
    }

    public ModuleInstanceKey(String moduleId, String moduleClass)
    {
        this.moduleId = moduleId;
        this.moduleClass = moduleClass;
    }
}
