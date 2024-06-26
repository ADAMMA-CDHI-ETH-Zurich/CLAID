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

import java.util.ArrayList;
import java.util.List;

import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.ModuleAnnotation;
import adamma.c4dhi.claid.TypeMapping.DataType;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.TypeMapping.Mutator;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;
import adamma.c4dhi.claid.PropertyHint;
import adamma.c4dhi.claid.PropertyType;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;


public class ModuleAnnotator {

    private ModuleAnnotation.Builder annotation;
    private String moduleType;

    public ModuleAnnotator(String moduleType) 
    {
        this.moduleType = moduleType;
        this.annotation = ModuleAnnotation.newBuilder();
    }

    public void setModuleDescription(String moduleDescription) 
    {
        this.annotation = this.annotation.setModuleDescription(moduleDescription);
    }

    public void setModuleCategory(String moduleCategory)
    {
        this.annotation.setModuleCategory(moduleCategory);
    }

    public PropertyHint makeDefaultProperty()
    {
        PropertyHint.Builder propertyHint = PropertyHint.newBuilder();
        propertyHint.setPropertyType(PropertyType.PROPERTY_TYPE_DEFAULT);
        return propertyHint.build();
    }

    public PropertyHint makeEnumProperty(String[] enumValues)
    {
        PropertyHint.Builder propertyHint = PropertyHint.newBuilder();
        propertyHint.setPropertyType(PropertyType.PROPERTY_TYPE_ENUM);

        for(String value : enumValues)
        {
            propertyHint.addPropertyTypeEnumValues(value);
        }   
        return propertyHint.build();
    }

    public PropertyHint makeIntegerProperty(long min, long max)
    {
        PropertyHint.Builder propertyHint = PropertyHint.newBuilder();
        propertyHint.setPropertyType(PropertyType.PROPERTY_TYPE_INT);
        propertyHint.setPropertyTypeIntMin(min);
        propertyHint.setPropertyTypeIntMax(max);
        return propertyHint.build();
    }

    public void describeProperty(String propertyName, String propertyDescription) 
    {
        describeProperty(propertyName, propertyDescription, makeDefaultProperty());
    }


    public void describeProperty(String propertyName, String propertyDescription, PropertyHint propertyHint) 
    {
        if(propertyName == null){
            return;
        }
        if(propertyDescription == null)
        {
            propertyDescription = "";
        }
        Logger.logInfo("Describe property 1 " + propertyName);
        this.annotation.addProperties(propertyName);
        Logger.logInfo("Describe property 2 " + propertyDescription);

        this.annotation.addPropertyDescriptions(propertyDescription);
        this.annotation.addPropertyHints(propertyHint);
        Logger.logInfo("Describe property 3 " + propertyDescription);

    }

    private void describePublishChannelDataType(String channelName, DataType dataType, String channelDescription) 
    {
        if(channelName == null){
            return;
        }
        if(channelDescription == null)
        {
            channelDescription = "";
        }
        DataPackage examplePackage = prepareExamplePackage(dataType, moduleType, channelName, true);
        this.annotation.addChannelDefinition(examplePackage);
        this.annotation.addChannelDescription(channelDescription);
    }
    
    public static class ShortType {}
    public static class IntegerType {}
    public static class LongType {}
    public static class FloatType {}
    public static class DoubleType {}
    public static class StringType {}

    public<T> void describePublishChannel(final String channelName, Class<T> dataType, final String channelDescription)
    {
        describePublishChannelDataType(channelName, new DataType(dataType), channelDescription);
    }

    public void describePublishChannel(final String channelName, ArrayList<Short> shorts, final String channelDescription, ShortType... ignore) 
    {
        describePublishChannelDataType(channelName, new DataType(ArrayList.class, "ArrayList<Short>"), channelDescription);
    }

    public void describePublishChannel(final String channelName, ArrayList<Integer> ints, final String channelDescription, IntegerType... ignore) 
    {
        describePublishChannelDataType(channelName, new DataType(ArrayList.class, "ArrayList<Integer>"), channelDescription);
    }

    public void describePublishChannel(final String channelName, ArrayList<Long> longs, final String channelDescription, LongType... ignore) 
    {
        describePublishChannelDataType(channelName, new DataType(ArrayList.class, "ArrayList<Long>"), channelDescription);
    }

    public void describePublishChannel(final String channelName, ArrayList<Float> floats, final String channelDescription, FloatType... ignore) 
    {
        describePublishChannelDataType(channelName, new DataType(ArrayList.class, "ArrayList<Float>"), channelDescription);
    }

    public void describePublishChannel(final String channelName, ArrayList<Double> dbs, final String channelDescription, DoubleType... ignore) 
    {
        describePublishChannelDataType(channelName, new DataType(ArrayList.class, "ArrayList<Double>"), channelDescription);
    }

    public void describePublishChannel(final String channelName, ArrayList<String> strings, final String channelDescription, StringType... ignore) 
    {
        describePublishChannelDataType(channelName, new DataType(ArrayList.class, "ArrayList<String>"), channelDescription);
    }

    
    // MAP
    public void describePublishChannel(final String channelName, Map<String, Short> shorts, final String channelDescription, ShortType... ignore) 
    {
        describePublishChannelDataType(channelName, new DataType(Map.class, "Map<String, Short>"), channelDescription);
    }

    public void describePublishChannel(final String channelName, Map<String, Integer> ints, final String channelDescription, IntegerType... ignore) 
    {
        describePublishChannelDataType(channelName, new DataType(Map.class, "Map<String, Integer>"), channelDescription);
    }

    public void describePublishChannel(final String channelName, Map<String, Long> longs, final String channelDescription, LongType... ignore) 
    {
        describePublishChannelDataType(channelName, new DataType(Map.class, "Map<String, Long>"), channelDescription);
    }

    public void describePublishChannel(final String channelName, Map<String, Float> floats, final String channelDescription, FloatType... ignore) 
    {
        describePublishChannelDataType(channelName, new DataType(Map.class, "Map<String, Float>"), channelDescription);
    }

    public void describePublishChannel(final String channelName, Map<String, Double> dbs, final String channelDescription, DoubleType... ignore) 
    {
        describePublishChannelDataType(channelName, new DataType(Map.class, "Map<String, Double>"), channelDescription);
    }

    public void describePublishChannel(final String channelName, Map<String, String> strings, final String channelDescription, StringType... ignore) 
    {
        describePublishChannelDataType(channelName, new DataType(Map.class, "Map<String, String>"), channelDescription);
    }


    // =================== Subscribe ===================

    public void describeSubscribeChannelDataType(String channelName, DataType dataType, String channelDescription) 
    {
        DataPackage examplePackage = prepareExamplePackage(dataType, moduleType, channelName, false);
        this.annotation.addChannelDefinition(examplePackage);
        this.annotation.addChannelDescription(channelDescription);
    }   

    public<T> T describeSubscribeChannel(final String channelName, Class<T> dataType, final String channelDescription)
    {
        describeSubscribeChannelDataType(channelName, new DataType(dataType), channelDescription);
        return null;
    }

    public void describeSubscribeChannel(final String channelName, ArrayList<Short> shorts, final String channelDescription, ShortType... ignore) 
    {
        describeSubscribeChannelDataType(channelName, new DataType(ArrayList.class, "ArrayList<Short>"), channelDescription);
    }

    public void describeSubscribeChannel(final String channelName, ArrayList<Integer> ints, final String channelDescription, IntegerType... ignore) 
    {
        describeSubscribeChannelDataType(channelName, new DataType(ArrayList.class, "ArrayList<Integer>"), channelDescription);
    }

    public void describeSubscribeChannel(final String channelName, ArrayList<Long> longs, final String channelDescription, LongType... ignore) 
    {
        describeSubscribeChannelDataType(channelName, new DataType(ArrayList.class, "ArrayList<Long>"), channelDescription);
    }

    public void describeSubscribeChannel(final String channelName, ArrayList<Float> floats, final String channelDescription, FloatType... ignore) 
    {
        describeSubscribeChannelDataType(channelName, new DataType(ArrayList.class, "ArrayList<Float>"), channelDescription);
    }

    public void describeSubscribeChannel(final String channelName, ArrayList<Double> dbs, final String channelDescription, DoubleType... ignore) 
    {
        describeSubscribeChannelDataType(channelName, new DataType(ArrayList.class, "ArrayList<Double>"), channelDescription);
    }

    public void describeSubscribeChannel(final String channelName, ArrayList<String> strings, final String channelDescription, StringType... ignore) 
    {
        describeSubscribeChannelDataType(channelName, new DataType(ArrayList.class, "ArrayList<String>"), channelDescription);
    }

    
    // MAP
    public void describeSubscribeChannel(final String channelName, Map<String, Short> shorts, final String channelDescription, ShortType... ignore) 
    {
        describeSubscribeChannelDataType(channelName, new DataType(Map.class, "Map<String, Short>"), channelDescription);
    }

    public void describeSubscribeChannel(final String channelName, Map<String, Integer> ints, final String channelDescription, IntegerType... ignore) 
    {
        describeSubscribeChannelDataType(channelName, new DataType(Map.class, "Map<String, Integer>"), channelDescription);
    }

    public void describeSubscribeChannel(final String channelName, Map<String, Long> longs, final String channelDescription, LongType... ignore) 
    {
        describeSubscribeChannelDataType(channelName, new DataType(Map.class, "Map<String, Long>"), channelDescription);
    }

    public void describeSubscribeChannel(final String channelName, Map<String, Float> floats, final String channelDescription, FloatType... ignore) 
    {
        describeSubscribeChannelDataType(channelName, new DataType(Map.class, "Map<String, Float>"), channelDescription);
    }

    public void describeSubscribeChannel(final String channelName, Map<String, Double> dbs, final String channelDescription, DoubleType... ignore) 
    {
        describeSubscribeChannelDataType(channelName, new DataType(Map.class, "Map<String, Double>"), channelDescription);
    }

    public void describeSubscribeChannel(final String channelName, Map<String, String> strings, final String channelDescription, StringType... ignore) 
    {
        describeSubscribeChannelDataType(channelName, new DataType(Map.class, "Map<String, String>"), channelDescription);
    }

    public ModuleAnnotation getAnnotations() 
    {
        return this.annotation.build();
    }

    public<T> DataPackage prepareExamplePackage(DataType dataType, final String moduleId, final String channelName, boolean isPublisher)
    {
        DataPackage.Builder builder = DataPackage.newBuilder();
        builder.setChannel(channelName);

        if(isPublisher)
        {
            // Only set module Id, host will be added by Middleware later.
            builder.setSourceModule(moduleId);//concatenateHostModuleAddress(this.host, moduleId));
        }
        else
        {
            // Only set module Id, host will be added by Middleware later.
            builder.setTargetModule(moduleId);//concatenateHostModuleAddress(this.host, moduleId));
        }
        DataPackage dataPackage = builder.build();

        T exampleInstance = TypeMapping.getNewInstance(dataType);
        if(exampleInstance == null)
        {
            System.out.println("Failed to create example instance for object of class \"" + dataType.getName() + "\".");
            return null;
        }

        Mutator<T> mutator = TypeMapping.getMutator(dataType);
        if(mutator == null)
        {
            System.out.println("Failed to create get mutator for object of class \"" + dataType.getName() + "\".");
            return null;
        }

        dataPackage = mutator.setPackagePayload(dataPackage, exampleInstance);

        return dataPackage;
    }
}
