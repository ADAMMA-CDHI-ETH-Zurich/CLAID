package adamma.c4dhi.claid.Module;

import java.util.ArrayList;
import java.util.List;

import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.ModuleAnnotation;
import adamma.c4dhi.claid.TypeMapping.DataType;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.TypeMapping.Mutator;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;

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

    public void describeProperty(String propertyName, String propertyDescription) 
    {
        this.annotation = this.annotation.addProperties(propertyName).addPropertyDescriptions(propertyDescription);
    }

    public void describePublishChannel(String channelName, DataType dataType, String channelDescription) 
    {
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
        describePublishChannel(channelName, new DataType(dataType), channelDescription);
    }

    protected void describePublishChannel(final String channelName, ArrayList<Short> shorts, final String channelDescription, ShortType... ignore) 
    {
        describePublishChannel(channelName, new DataType(ArrayList.class, "ArrayList<Short>"), channelDescription);
    }

    protected void describePublishChannel(final String channelName, ArrayList<Integer> ints, final String channelDescription, IntegerType... ignore) 
    {
        describePublishChannel(channelName, new DataType(ArrayList.class, "ArrayList<Integer>"), channelDescription);
    }

    protected void describePublishChannel(final String channelName, ArrayList<Long> longs, final String channelDescription, LongType... ignore) 
    {
        describePublishChannel(channelName, new DataType(ArrayList.class, "ArrayList<Long>"), channelDescription);
    }

    protected void describePublishChannel(final String channelName, ArrayList<Float> floats, final String channelDescription, FloatType... ignore) 
    {
        describePublishChannel(channelName, new DataType(ArrayList.class, "ArrayList<Float>"), channelDescription);
    }

    protected void describePublishChannel(final String channelName, ArrayList<Double> dbs, final String channelDescription, DoubleType... ignore) 
    {
        describePublishChannel(channelName, new DataType(ArrayList.class, "ArrayList<Double>"), channelDescription);
    }

    protected void describePublishChannel(final String channelName, ArrayList<String> strings, final String channelDescription, StringType... ignore) 
    {
        describePublishChannel(channelName, new DataType(ArrayList.class, "ArrayList<String>"), channelDescription);
    }

    
    // MAP
    protected void describePublishChannel(final String channelName, Map<String, Short> shorts, final String channelDescription, ShortType... ignore) 
    {
        describePublishChannel(channelName, new DataType(Map.class, "Map<String, Short>"), channelDescription);
    }

    protected void describePublishChannel(final String channelName, Map<String, Integer> ints, final String channelDescription, IntegerType... ignore) 
    {
        describePublishChannel(channelName, new DataType(Map.class, "Map<String, Integer>"), channelDescription);
    }

    protected void describePublishChannel(final String channelName, Map<String, Long> longs, final String channelDescription, LongType... ignore) 
    {
        describePublishChannel(channelName, new DataType(Map.class, "Map<String, Long>"), channelDescription);
    }

    protected void describePublishChannel(final String channelName, Map<String, Float> floats, final String channelDescription, FloatType... ignore) 
    {
        describePublishChannel(channelName, new DataType(Map.class, "Map<String, Float>"), channelDescription);
    }

    protected void describePublishChannel(final String channelName, Map<String, Double> dbs, final String channelDescription, DoubleType... ignore) 
    {
        describePublishChannel(channelName, new DataType(Map.class, "Map<String, Double>"), channelDescription);
    }

    protected void describePublishChannel(final String channelName, Map<String, String> strings, final String channelDescription, StringType... ignore) 
    {
        describePublishChannel(channelName, new DataType(Map.class, "Map<String, String>"), channelDescription);
    }


    // =================== Subscribe ===================

    public void describeSubscribeChannel(String channelName, DataType dataType, String channelDescription) 
    {
        DataPackage examplePackage = prepareExamplePackage(dataType, moduleType, channelName, false);
        this.annotation.addChannelDefinition(examplePackage);
        this.annotation.addChannelDescription(channelDescription);
    }   

    public<T> T describeSubscribeChannel(final String channelName, Class<T> dataType, final String channelDescription)
    {
        describeSubscribeChannel(channelName, new DataType(dataType), channelDescription);
        return null;
    }

    protected void describeSubscribeChannel(final String channelName, ArrayList<Short> shorts, final String channelDescription, ShortType... ignore) 
    {
        describeSubscribeChannel(channelName, new DataType(ArrayList.class, "ArrayList<Short>"), channelDescription);
    }

    protected void describeSubscribeChannel(final String channelName, ArrayList<Integer> ints, final String channelDescription, IntegerType... ignore) 
    {
        describeSubscribeChannel(channelName, new DataType(ArrayList.class, "ArrayList<Integer>"), channelDescription);
    }

    protected void describeSubscribeChannel(final String channelName, ArrayList<Long> longs, final String channelDescription, LongType... ignore) 
    {
        describeSubscribeChannel(channelName, new DataType(ArrayList.class, "ArrayList<Long>"), channelDescription);
    }

    protected void describeSubscribeChannel(final String channelName, ArrayList<Float> floats, final String channelDescription, FloatType... ignore) 
    {
        describeSubscribeChannel(channelName, new DataType(ArrayList.class, "ArrayList<Float>"), channelDescription);
    }

    protected void describeSubscribeChannel(final String channelName, ArrayList<Double> dbs, final String channelDescription, DoubleType... ignore) 
    {
        describeSubscribeChannel(channelName, new DataType(ArrayList.class, "ArrayList<Double>"), channelDescription);
    }

    protected void describeSubscribeChannel(final String channelName, ArrayList<String> strings, final String channelDescription, StringType... ignore) 
    {
        describeSubscribeChannel(channelName, new DataType(ArrayList.class, "ArrayList<String>"), channelDescription);
    }

    
    // MAP
    protected void describeSubscribeChannel(final String channelName, Map<String, Short> shorts, final String channelDescription, ShortType... ignore) 
    {
        describeSubscribeChannel(channelName, new DataType(Map.class, "Map<String, Short>"), channelDescription);
    }

    protected void describeSubscribeChannel(final String channelName, Map<String, Integer> ints, final String channelDescription, IntegerType... ignore) 
    {
        describeSubscribeChannel(channelName, new DataType(Map.class, "Map<String, Integer>"), channelDescription);
    }

    protected void describeSubscribeChannel(final String channelName, Map<String, Long> longs, final String channelDescription, LongType... ignore) 
    {
        describeSubscribeChannel(channelName, new DataType(Map.class, "Map<String, Long>"), channelDescription);
    }

    protected void describeSubscribeChannel(final String channelName, Map<String, Float> floats, final String channelDescription, FloatType... ignore) 
    {
        describeSubscribeChannel(channelName, new DataType(Map.class, "Map<String, Float>"), channelDescription);
    }

    protected void describeSubscribeChannel(final String channelName, Map<String, Double> dbs, final String channelDescription, DoubleType... ignore) 
    {
        describeSubscribeChannel(channelName, new DataType(Map.class, "Map<String, Double>"), channelDescription);
    }

    protected void describeSubscribeChannel(final String channelName, Map<String, String> strings, final String channelDescription, StringType... ignore) 
    {
        describeSubscribeChannel(channelName, new DataType(Map.class, "Map<String, String>"), channelDescription);
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
