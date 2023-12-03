package adamma.c4dhi.claid.TypeMapping;

// Describe a data type that can be send via a Channel
public class DataType 
{
    private Class<?> type;
    // If type is a Collection, e.g., ArrayList<String>, genericTypeName will contain the name of the whole type (in this case: "ArrayList<String>")
    private String genericTypeName; 
    boolean generic = false;

    public DataType(Class<?> type)
    {
        this.type = type;
        this.genericTypeName = "";
        this.generic = false;
    }

    public DataType(Class<?> type, final String genericTypeName)
    {
        this.type = type;
        this.genericTypeName = genericTypeName;
        this.generic = true;
    }

    public String getName()
    {
        if(this.generic)
        {
            return this.genericTypeName;
        }
        else
        {
            return this.type.getName();
        }
    }

    public String getSimpleName()
    {
        if(this.generic)
        {
            return this.genericTypeName;
        }
        else
        {
            return this.type.getSimpleName();
        }
    }

    public boolean isGeneric()
    {
        return this.generic;
    }

    public boolean equals(DataType other)
    {
        if(this.isGeneric() && other.isGeneric())
        {
            return true;
        }
        else if(!this.isGeneric() && !other.isGeneric())
        {
            return this.getClass().equals(other.getClass());
        }
        else
        {
            return false;
        }
    }

    public Class<?> getDataTypeClass()
    {
        return this.type;
    }
}
