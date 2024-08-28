/***************************************************************************
* Copyright (C) 2023 ETH Zurich
* CLAID: Closing the Loop on AI & Data Collection (https://claid.ethz.ch)
* Core AI & Digital Biomarker, Acoustic and Inflammatory Biomarkers (ADAMMA)
* Centre for Digital Health Interventions (c4dhi.org)
* 
* Authors: Patrick Langer
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
*         http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
***************************************************************************/

package adamma.c4dhi.claid.Module;

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import adamma.c4dhi.claid.Logger.Logger;

import java.util.List;
import java.util.StringTokenizer;
import java.lang.reflect.Method;

import com.google.protobuf.Struct;
import com.google.protobuf.Value;
import com.google.protobuf.Message;
import com.google.protobuf.MessageOrBuilder;
import com.google.protobuf.util.JsonFormat;
import com.google.protobuf.GeneratedMessageV3;

public class Properties 
{
    private final Struct properties;
    private final List<String> unknownProperties;

    public Properties(Struct properties) 
    {
        this.properties = properties;
        this.unknownProperties = new Vector<>();
    }

  

    public <T> T getNumberProperty(String key, Class<T> dataType) 
    {
        Logger.logInfo("getProperty " + properties);
        Value property = lookupProperty(key);


        if (property != null) 
        {
            Double propertyValue = 0.0;

            if (property.getKindCase() == Value.KindCase.NUMBER_VALUE) 
            {
                propertyValue = property.getNumberValue();
            } 
            else if (property.getKindCase() == Value.KindCase.STRING_VALUE) 
            {
                propertyValue = Double.valueOf(property.getStringValue());
            } 
            Logger.logInfo("Get property 1");
            Logger.logInfo("Get property 2");

            T value;
            Logger.logInfo("Get property 3 ");

            try {
                if (dataType.equals(Double.class)) 
                {
                    Logger.logInfo("Get property 4");
                    value = (T) propertyValue;
                } 
                else if (dataType.equals(Float.class)) 
                {
                    Logger.logInfo("Get property 5");

                    value = (T) Float.valueOf(propertyValue.floatValue());
                } 
                else if (dataType.equals(Long.class)) 
                {
                    Logger.logInfo("Get property 6");

                    value = (T) Long.valueOf(propertyValue.longValue());
                } 
                else if (dataType.equals(Integer.class)) 
                {
                    Logger.logInfo("Get property 7");

                    value = (T) Integer.valueOf(propertyValue.intValue());
                } 
                else if (dataType.equals(Short.class)) 
                {
                    Logger.logInfo("Get property 8");

                    value = (T) Short.valueOf(propertyValue.shortValue());
                } 
                else if (dataType.equals(Byte.class)) 
                {
                    Logger.logInfo("Get property 9");

                    value = (T) Byte.valueOf(propertyValue.byteValue());
                } 
                
                
                else 
                {
                    Logger.logInfo("Get property 10");

                    this.unknownProperties.add(key);
                    return null;
                }
                return value;
            } 
            catch (NumberFormatException e) 
            {
                Logger.logError("Failed to parse property value \"" + key + "\": "+ e.getMessage());
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
            else
            {
                return null;
            }
        }
    }

    public String getStringProperty(String key) 
    {
        Value property;
        property = lookupProperty(key);
        if (property != null && property.getKindCase() == Value.KindCase.STRING_VALUE) 
        {
            return property.getStringValue();
        } 
        else 
        {
            unknownProperties.add(key);
            return "";
        }
    }

    public String getStringProperty(String key, String defaultValue) 
    {
        Value property;
        property = lookupProperty(key);
        if (property != null && property.getKindCase() == Value.KindCase.STRING_VALUE) 
        {
            return property.getStringValue();
        } 
        return defaultValue;
    }

    public boolean getBoolProperty(String key) 
    {
        Value property;
        property = lookupProperty(key);
        if (property != null && property.getKindCase() == Value.KindCase.BOOL_VALUE) 
        {
            return property.getBoolValue();
        } 
        else 
        {
            unknownProperties.add(key);
            return false;
        }
    }

    public boolean getBoolProperty(String key, boolean defaultValue) 
    {
        Value property;
        property = lookupProperty(key);
        if (property != null && property.getKindCase() == Value.KindCase.BOOL_VALUE) 
        {
            return property.getBoolValue();
        } 
        return defaultValue;
    }

    public <T extends Message> T getObjectProperty(String key, Class<T> dataType)
    {
        Value property;
        property = lookupProperty(key);
        if(property == null)
        {
            unknownProperties.add(key);
            return null;
        }
        
        return deserializeObjectProperty(key, property, dataType);
    }

    public <T extends Message> getObjectProperty(String key, Class<T> dataType, T defaultValue)
    {
        Value property;
        property = lookupProperty(key);
        if(property == null)
        {
            // Property not found, return default value.
            return defaultValue;
        }
        
        return deserializeObjectProperty(key, property, dataType);
    }

    private <T extends Message> T deserializeObjectProperty(String key, Value property, Class<T> dataType)
    {
        String jsonString;
        try
        {
            jsonString = JsonFormat.printer().print(property);
        }
        catch(Exception e)
        {
            Logger.logError("Properties getObjectProperty() failed, unable to serialize property object to json " + e.getMessage() + " " + e.getCause());
            unknownProperties.add(key);
            return null;
        }

        GeneratedMessageV3.Builder builder = Properties.getProtoMessageBuilder(dataType);
        if(builder == null)
        {
            Logger.logError("Failed to create builder for proto type " + dataType.getName());
            unknownProperties.add(key);
            return null;
        }

        try
        {
            JsonFormat.parser().ignoringUnknownFields().merge(jsonString, builder);
        }
        catch(Exception e)
        {
            Logger.logError("Properties getObjectProperty() failed, unable to deserialize json into type \"" + dataType.getName() + ": " + e.getMessage() + " " + e.getCause());
            unknownProperties.add(key);
            return null;
        }
        
        return (T) builder.build();
    }

    private static <T> GeneratedMessageV3.Builder getProtoMessageBuilder(Class<T> dataType)
    {
        try 
        {
            Method newBuilderMethod = dataType.getMethod("newBuilder");
            if (newBuilderMethod != null) 
            {
                Object untypedBuilder = newBuilderMethod.invoke(null);
                if (untypedBuilder instanceof GeneratedMessageV3.Builder) 
                {
                    GeneratedMessageV3.Builder builder = (GeneratedMessageV3.Builder) untypedBuilder;
                    return builder;
                }
            }
        }
        catch (Exception e) 
        {
            Logger.logError("Properties error: " + e.getMessage()); // Handle the exception appropriately
        }
        return null;
    }

    public boolean wasAnyPropertyUnknown() 
    {
        return !this.unknownProperties.isEmpty();
    }

    public List<String> getUnknownProperties() {
        return this.unknownProperties;
    }

    public boolean hasProperty(String key)
    {
        return this.properties.getFieldsMap().containsKey(key);
    }

    public String getMissingPropertiesErrorString()
    {
        return "Missing properties: [" + unknownPropertiesToString() +  "]. Please sepcify the properties in the configuration file.";
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

    private Value lookupProperty(String key) 
    {
        if (properties.getFieldsMap().containsKey(key)) 
        {
            Value property = properties.getFieldsMap().get(key);
            return property;
        } 
        else 
        {
            return null;
        }
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