package adamma.c4dhi.claid.Module.PropertyHelper;

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import adamma.c4dhi.claid.Logger.Logger;

import java.util.List;
import java.util.StringTokenizer;

public class PropertyHelper 
{
    private final Map<String, String> properties;
    private final List<String> unknownProperties;

    public PropertyHelper(final Map<String, String> properties) 
    {
        this.properties = properties;
        this.unknownProperties = new Vector<>();
    }

  

    public <T> T getProperty(final String key, Class<T> dataType) {
        Logger.logInfo("getProperty " + properties);
        if (properties.containsKey(key)) {
            Logger.logInfo("Get property 1");
            String propertyValue = properties.get(key);
            Logger.logInfo("Get property 2");

            T value;
            Logger.logInfo("Get property 3");

            try {
                if (dataType.equals(Double.class)) 
                {
                    value = (T) Double.valueOf(propertyValue);
                } 
                else if (dataType.equals(Float.class)) 
                {
                    value = (T) Float.valueOf(propertyValue);
                } 
                else if (dataType.equals(Long.class)) 
                {
                    value = (T) Long.valueOf(propertyValue);
                } 
                else if (dataType.equals(Integer.class)) 
                {
                    value = (T) Integer.valueOf(propertyValue);
                } 
                else if (dataType.equals(Short.class)) 
                {
                    value = (T) Short.valueOf(propertyValue);
                } 
                else if (dataType.equals(Byte.class)) 
                {
                    value = (T) Byte.valueOf(propertyValue);
                } 
                else if(dataType.equals(Boolean.class))
                {
                    Logger.logInfo("Get property 3");

                    value = (T) Boolean.valueOf(propertyValue);
                    Logger.logInfo("Get property 4");

                }
                else if (dataType.equals(String.class)) 
                {
                    value = (T) propertyValue;
                } 
                else 
                {
                    this.unknownProperties.add(key);
                    return null;
                }
                return value;
            } 
            catch (NumberFormatException e) 
            {
                this.unknownProperties.add(key);
                return null;
            }
        } else {
            this.unknownProperties.add(key);
            // Return a default value, otherwise user might get NullPointerException.
            if (dataType.equals(Double.class)) 
            {
                return (T) Double.valueOf(0);
            } 
            else if (dataType.equals(Float.class)) 
            {
                return (T) Float.valueOf(0);
            } 
            else if (dataType.equals(Long.class)) 
            {
                return (T) Long.valueOf(0);
            } 
            else if (dataType.equals(Integer.class)) 
            {
                return (T) Integer.valueOf(0);
            } 
            else if (dataType.equals(Short.class)) 
            {
                return (T) Short.valueOf((short) 0);
            } 
            else if (dataType.equals(Byte.class)) 
            {
                return (T) Byte.valueOf((byte) 0);
            } 
            else if(dataType.equals(Boolean.class))
            {
                return (T) Boolean.valueOf(false);

            }
            else if (dataType.equals(String.class)) 
            {
                return (T) "";
            } 
            else
            {
                return null;
            }
        }
    }

    public boolean wasAnyPropertyUnknown() {
        return !this.unknownProperties.isEmpty();
    }

    public List<String> getUnknownProperties() {
        return this.unknownProperties;
    }

    public String unknownPropertiesToString() {
        String returnValue = new String();

        for (String property : this.unknownProperties) {
            if (returnValue.length() == 0) {
                returnValue += property;
            } else {
                returnValue += ", " + property;
            }
        }
        return returnValue;
    }
}


// public static class ShortType {}
// public static class IntegerType {}
// public static class LongType {}
// public static class FloatType {}
// public static class DoubleType {}
// public static class StringType {}

// public Short getProperty(final String key, Class<Short> dataType, ShortType... ignore)
// {
//     if (properties.containsKey(key)) 
//     {
//         String propertyValue = properties.get(key); 
        
//         try
//         {
//             Short value = Short.valueOf(propertyValue);
//         }
//         catch (NumberFormatException e) 
//         {
//             Logger.logError(e.getMessage());
//             e.printStackTrace();
//             this.unknownProperties.add(key);
//             return null;
//         }
        
//     }
//     else
//     {
//         this.unknownProperties.add(key);
//         return null;
//     }
// }