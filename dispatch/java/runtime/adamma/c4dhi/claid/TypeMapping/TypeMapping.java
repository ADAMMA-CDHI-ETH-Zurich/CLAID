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

package adamma.c4dhi.claid.TypeMapping;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;


import adamma.c4dhi.claid.TypeMapping.Mutator;

import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.Blob;
import adamma.c4dhi.claid.TracePoint;
import adamma.c4dhi.claid.StringArray;
import adamma.c4dhi.claid.NumberArray;
import adamma.c4dhi.claid.NumberMap;
import adamma.c4dhi.claid.StringMap;


import com.google.protobuf.GeneratedMessageV3;


public class TypeMapping {

    public static DataPackage.Builder dataPackageBuilderCopy(DataPackage p)
    {
        DataPackage.Builder builder = DataPackage.newBuilder()
                    .setId(p.getId())
                    .setChannel(p.getChannel())
                    .setSourceModule(p.getSourceModule())
                    .setTargetModule(p.getTargetModule())
                    .setSourceHost(p.getSourceHost())
                    .setTargetHost(p.getTargetHost())
                    .setSourceUserToken(p.getSourceUserToken())
                    .setTargetUserToken(p.getTargetUserToken())
                    .setUnixTimestampMs(p.getUnixTimestampMs())
                    .setDeviceId(p.getDeviceId());

        List<TracePoint> tracePoints = p.getTracePointsList();
        for(int i = 0; i < tracePoints.size(); i++)
        {
            builder.setTracePoints(i, tracePoints.get(i));
        }
        return builder;
    }

    public static <T> T getProtoMessageInstance(Class<T> dataType)
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
                    return (T) builder.build();
                }
            }
        } catch (Exception e) {
            e.printStackTrace(); // Handle the exception appropriately
        }
        return null;
    }

    // Can be used to create a new instance of an object of Class<T>.
    // If T is not a protobuf message, we look up the declared constructor from the class and create a new instance of the Class via that constructor.
    // 
    // If T is a protobuf message, we use getProtoMessageInstance to get a new instance of that type.
    // In Java, the constructor of Protobuf classes is private. Hence, an object as instance of a protobuf class can not 
    // be created via using the constructor (i.e. new ProtoBufType() does not work, and we can also not use the declared constructor).
    // The instance of the Protobuf type has to be created via the Builder. Each Protobuf class has a newBuilder() function, that can
    // be used to get a new builder. Using the Class object, we can look up the correct builder and use it to retrieve an instance.
    public static <T> T getNewInstance(Class<T> clz)
    {
        try
        {
            if (GeneratedMessageV3.class.isAssignableFrom(clz)) 
            {
                T instance = getProtoMessageInstance(clz);
                return instance;
            }
            else if (clz == Double.class) {
                // For Double, create an instance with a default value of 0.0
                Double instance = 0.0;
                return (T) instance;
            } else if (clz == Integer.class) {
                // For Integer, create an instance with a default value of 0
                Integer instance = 0;
                return (T) instance;
            } 
            else if (clz == Short.class) {
                // For Long, create an instance with a default value of 0L
                Short instance = 0;
                return (T) instance;
            } 
            else if (clz == Long.class) {
                // For Long, create an instance with a default value of 0L
                Long instance = 0L;
                return (T) instance;
            } else if (clz == Float.class) {
                // For Float, create an instance with a default value of 0.0f
                Float instance = 0.0f;
                return (T) instance;
            } else if (clz == Boolean.class) {
                // For Boolean, create an instance with a default value of false
                Boolean instance = false;
                return (T) instance;
            } 
            else
            {
                T instance = clz.getDeclaredConstructor().newInstance();
                return instance;
            }
        }
        catch (NoSuchMethodException | InstantiationException | IllegalAccessException | InvocationTargetException e) {
            System.out.println("TypeMapping.getNewInstance(): Unsupported data type");
            e.printStackTrace();
            return null;
        }

    }   
    
    public static<T> T getNewInstance(DataType dataType)
    {
        
        if(!dataType.isGeneric())
        {
            Class<?> dataTypeClass = dataType.getDataTypeClass();
            return (T) getNewInstance(dataTypeClass);
        }
        else
        {
            // If it is a generic class, e.g., ArrayList<String>,
            // normally we could not find oujt the data type (i.e., String), 
            // because of the world's greatest invention "type erasure"..
            // However, with our custom DataType class, we set the name of the generic type manually
            // when creating the object. Check the Publish and Subscribe functions of Module for example.
            String typeName = dataType.getName();
            
            if(typeName.equals("ArrayList<String>"))
            {
                return (T) new ArrayList<String>();
            }
            else if (typeName.equals("ArrayList<Short>")) 
            {
                return (T) new ArrayList<Short>();
            } else if (typeName.equals("ArrayList<Integer>")) 
            {
                return (T) new ArrayList<Integer>();
            } 
            else if (typeName.equals("ArrayList<Long>")) 
            {
                return (T) new ArrayList<Long>();
            } 
            else if (typeName.equals("ArrayList<Float>")) 
            {
                return (T) new ArrayList<Float>();
            } 
            else if (typeName.equals("ArrayList<Double>")) 
            {
                return (T) new ArrayList<Double>();
            } 
            else if (typeName.equals("Map<String, Short>")) 
            {
                return (T) new HashMap<String, Short>();
            } 
            else if (typeName.equals("Map<String, Integer>")) 
            {
                return (T) new HashMap<String, Integer>();
            } 
            else if (typeName.equals("Map<String, Long>")) 
            {
                return (T) new HashMap<String, Long>();
            } 
            else if (typeName.equals("Map<String, Float>")) 
            {
                return (T) new HashMap<String, Float>();
            } 
            else if (typeName.equals("Map<String, Double>")) 
            {
                return (T) new HashMap<String, Double>();
            }
            else if (typeName.equals("Map<String, String>")) 
            {
                return (T) new HashMap<String, String>();
            } 
            else 
            {
                // Handle other cases or return null if needed
                return null;
            }
            
        }


    }

    private static HashMap<String, ProtoCodec> protoCodecMap = new HashMap<>();

    private static ProtoCodec getProtoCodec(GeneratedMessageV3 msg) 
    {
      final String fullName =  msg.getDescriptorForType().getFullName();
      
      if(!protoCodecMap.containsKey(fullName))
      {
        protoCodecMap.put(fullName, new ProtoCodec(msg));
      }

      return protoCodecMap.get(fullName);
    }
    

    public static <T> Mutator<T> getMutator(DataType dataType) 
    {
        Class<?> dataTypeClass = dataType.getDataTypeClass();

        if (dataTypeClass == Double.class || dataTypeClass == Float.class || 
        dataTypeClass == Integer.class || dataTypeClass == Short.class || dataTypeClass == Long.class) {
            return new Mutator<T>(
                (p, v) -> dataPackageBuilderCopy(p)
                    .setNumberVal((Double) v)
                    .build(),
                p -> (T) Double.valueOf(p.getNumberVal())
            );
        }

        if (dataTypeClass == Boolean.class) {
            return new Mutator<T>(
                (p, v) -> dataPackageBuilderCopy(p)
                    .setBoolVal((Boolean) v)
                    .build(),
                p -> (T) Boolean.valueOf(p.getBoolVal())
            );
        }

        if (dataTypeClass == String.class) {
            return new Mutator<T>(
                (p, v) -> dataPackageBuilderCopy(p)
                    .setStringVal((String) v)
                    .build(),
                p -> (T) p.getStringVal()
            );
        }
        

        // LIST / ARRAY
        if (dataType.isGeneric() && dataTypeClass == ArrayList.class) 
        {
            // getName returns a hardcoded string for generic types, which was
            // specified when creating this instance of DataType.
            String genericName = dataType.getName();
           
            if (genericName.equals("ArrayList<Short>")) 
            {
                return new Mutator<T>(
                    (p, v) -> {
                        ArrayList<Short> data = (ArrayList<Short>) v;
                        DataPackage.Builder builder = dataPackageBuilderCopy(p);
            
                        NumberArray.Builder numberArrayBuilder = NumberArray.newBuilder();
                        for (Short value : data) {
                            numberArrayBuilder.addVal(value.doubleValue());
                        }
            
                        builder.setNumberArrayVal(numberArrayBuilder.build());
                        return builder.build();
                    },
                    p -> {
                        ArrayList<Short> array = new ArrayList<>();
                        NumberArray numberArray = p.getNumberArrayVal();
            
                        for (Double value : numberArray.getValList()) {
                            array.add(value.shortValue());
                        }
                        
                        return (T) array;
                    }
                );
            } 
            else if (genericName.equals("ArrayList<Integer>")) 
            {
                return new Mutator<T>(
                    (p, v) -> {
                        ArrayList<Integer> data = (ArrayList<Integer>) v;
                        DataPackage.Builder builder = dataPackageBuilderCopy(p);
            
                        NumberArray.Builder numberArrayBuilder = NumberArray.newBuilder();
                        for (Integer value : data) {
                            numberArrayBuilder.addVal(value.doubleValue());
                        }
            
                        builder.setNumberArrayVal(numberArrayBuilder.build());
                        return builder.build();
                    },
                    p -> {
                        ArrayList<Integer> array = new ArrayList<>();
                        NumberArray numberArray = p.getNumberArrayVal();
            
                        for (Double value : numberArray.getValList()) {
                            array.add(value.intValue());
                        }
                        
                        
                        return (T) array;
                    }
                );
            } 
            else if (genericName.equals("ArrayList<Long>")) {
                return new Mutator<T>(
                    (p, v) -> {
                        ArrayList<Long> data = (ArrayList<Long>) v;
                        DataPackage.Builder builder = dataPackageBuilderCopy(p);
            
                        NumberArray.Builder numberArrayBuilder = NumberArray.newBuilder();
                        for (Long value : data) {
                            numberArrayBuilder.addVal(value);
                        }
            
                        builder.setNumberArrayVal(numberArrayBuilder.build());
                        return builder.build();
                    },
                    p -> {
                        ArrayList<Long> array = new ArrayList<>();
                        NumberArray numberArray = p.getNumberArrayVal();
            
                        for (Double value : numberArray.getValList()) {
                            array.add(value.longValue());
                        }
                        
                        
                        return (T) array;
                    }
                );
            } 
            else if (genericName.equals("ArrayList<Float>")) {
                return new Mutator<T>(
                    (p, v) -> {
                        ArrayList<Float> data = (ArrayList<Float>) v;
                        DataPackage.Builder builder = dataPackageBuilderCopy(p);
            
                        NumberArray.Builder numberArrayBuilder = NumberArray.newBuilder();
                        for (Float value : data) {
                            numberArrayBuilder.addVal(value);
                        }
            
                        builder.setNumberArrayVal(numberArrayBuilder.build());
                        return builder.build();
                    },
                    p -> {
                        ArrayList<Float> array = new ArrayList<>();
                        NumberArray numberArray = p.getNumberArrayVal();
            
                        for (Double value : numberArray.getValList()) {
                            array.add(value.floatValue());
                        }
                        
                        
                        return (T) array;
                    }
                );
            }
            else if(genericName.equals("ArrayList<Double>"))
            {
                return new Mutator<T>(
                    (p, v) -> {
                        ArrayList<Double> data = (ArrayList<Double>) v;
                        DataPackage.Builder builder = dataPackageBuilderCopy(p);

                        NumberArray.Builder numberArrayBuilder = NumberArray.newBuilder();
                        for(Double value : data)
                        {
                            numberArrayBuilder.addVal(value);
                        }

                        builder.setNumberArrayVal(numberArrayBuilder.build());
                        return builder.build();
                    },
                    p -> {
                        ArrayList<Double> array = new ArrayList<>();
                        NumberArray numberArray = p.getNumberArrayVal();

                        return (T) new ArrayList<Double>(numberArray.getValList());
                    }
                );
            }
            else if(genericName.equals("ArrayList<String>"))
            {
                return new Mutator<T>(
                    (p, v) -> {
                        ArrayList<String> data = (ArrayList<String>) v;
                        DataPackage.Builder builder = dataPackageBuilderCopy(p);

                        StringArray.Builder stringArrayBuilder = StringArray.newBuilder();
                        for(String value : data)
                        {
                            stringArrayBuilder.addVal(value);
                        }

                        builder.setStringArrayVal(stringArrayBuilder.build());
                        return builder.build();
                    },
                    p -> {
                        ArrayList<String> array = new ArrayList<>();
                        StringArray stringArray = p.getStringArrayVal();

                        return (T) new ArrayList<String>(stringArray.getValList());
                    }
                );
            }
        }

        // MAP
        if (dataType.isGeneric() && dataTypeClass == Map.class) 
        {
            // getName returns a hardcoded string for generic types, which was
            // specified when creating this instance of DataType.
            String genericName = dataType.getName();
           
            
            if (genericName.equals("Map<String, Short>")) 
            {
                return new Mutator<T>(
                    (p, v) -> {
                        Map<String, Short> data = (Map<String, Short>) v;
                        DataPackage.Builder builder = dataPackageBuilderCopy(p);

                        NumberMap.Builder numberMapBuilder = NumberMap.newBuilder();
                        for (Map.Entry<String, Short> entry : data.entrySet()) {
                            numberMapBuilder.putVal(entry.getKey(), entry.getValue().doubleValue());
                        }

                        builder.setNumberMap(numberMapBuilder.build());
                        return builder.build();
                    },
                    p -> {
                        Map<String, Short> map = new HashMap<>();
                        NumberMap numberMap = p.getNumberMap();

                        for (String key : numberMap.getValMap().keySet()) {
                            double value = numberMap.getValMap().get(key);
                            map.put(key, (short) value);
                        }

                        return (T) map;
                    }
                );
            } 

            if (genericName.equals("Map<String, Integer>")) 
            {
                return new Mutator<T>(
                    (p, v) -> {
                        Map<String, Integer> data = (Map<String, Integer>) v;
                        DataPackage.Builder builder = dataPackageBuilderCopy(p);

                        NumberMap.Builder numberMapBuilder = NumberMap.newBuilder();
                        for (Map.Entry<String, Integer> entry : data.entrySet()) {
                            numberMapBuilder.putVal(entry.getKey(), entry.getValue().doubleValue());
                        }

                        builder.setNumberMap(numberMapBuilder.build());
                        return builder.build();
                    },
                    p -> {
                        Map<String, Integer> map = new HashMap<>();
                        NumberMap numberMap = p.getNumberMap();

                        for (String key : numberMap.getValMap().keySet()) {
                            double value = numberMap.getValMap().get(key);
                            map.put(key, (int) value);
                        }

                        return (T) map;
                    }
                );
            } 

            if (genericName.equals("Map<String, Long>")) 
            {
                return new Mutator<T>(
                    (p, v) -> {
                        Map<String, Long> data = (Map<String, Long>) v;
                        DataPackage.Builder builder = dataPackageBuilderCopy(p);

                        NumberMap.Builder numberMapBuilder = NumberMap.newBuilder();
                        for (Map.Entry<String, Long> entry : data.entrySet()) {
                            numberMapBuilder.putVal(entry.getKey(), entry.getValue().doubleValue());
                        }

                        builder.setNumberMap(numberMapBuilder.build());
                        return builder.build();
                    },
                    p -> {
                        Map<String, Long> map = new HashMap<>();
                        NumberMap numberMap = p.getNumberMap();

                        for (String key : numberMap.getValMap().keySet()) {
                            double value = numberMap.getValMap().get(key);
                            map.put(key, (long) value);
                        }

                        return (T) map;
                    }
                );
            } 

            if (genericName.equals("Map<String, Float>")) 
            {
                return new Mutator<T>(
                    (p, v) -> {
                        Map<String, Float> data = (Map<String, Float>) v;
                        DataPackage.Builder builder = dataPackageBuilderCopy(p);

                        NumberMap.Builder numberMapBuilder = NumberMap.newBuilder();
                        for (Map.Entry<String, Float> entry : data.entrySet()) {
                            numberMapBuilder.putVal(entry.getKey(), entry.getValue().floatValue());
                        }

                        builder.setNumberMap(numberMapBuilder.build());
                        return builder.build();
                    },
                    p -> {
                        Map<String, Float> map = new HashMap<>();
                        NumberMap numberMap = p.getNumberMap();

                        for (String key : numberMap.getValMap().keySet()) {
                            double value = numberMap.getValMap().get(key);
                            map.put(key, (float) value);
                        }

                        return (T) map;
                    }
                );
            } 

            if (genericName.equals("Map<String, Double>")) 
            {
                return new Mutator<T>(
                    (p, v) -> {
                        Map<String, Double> data = (Map<String, Double>) v;
                        DataPackage.Builder builder = dataPackageBuilderCopy(p);

                        NumberMap.Builder numberMapBuilder = NumberMap.newBuilder();
                        for (Map.Entry<String, Double> entry : data.entrySet()) {
                            numberMapBuilder.putVal(entry.getKey(), entry.getValue());
                        }

                        builder.setNumberMap(numberMapBuilder.build());
                        return builder.build();
                    },
                    p -> {
                        Map<String, Double> map = new HashMap<>();
                        NumberMap numberMap = p.getNumberMap();

                        for (String key : numberMap.getValMap().keySet()) {
                            double value = numberMap.getValMap().get(key);
                            map.put(key, value);
                        }

                        return (T) map;
                    }
                );
            } 
            if (genericName.equals("Map<String, String>")) 
            {
                return new Mutator<T>(
                    (p, v) -> {
                        Map<String, String> data = (Map<String, String>) v;
                        DataPackage.Builder builder = dataPackageBuilderCopy(p);

                        StringMap.Builder stringMapBuilder = StringMap.newBuilder();
                        for (Map.Entry<String, String> entry : data.entrySet()) {
                            stringMapBuilder.putVal(entry.getKey(), entry.getValue());
                        }

                        builder.setStringMap(stringMapBuilder.build());
                        return builder.build();
                    },
                    p -> {
                        Map<String, String> map = new HashMap<>();
                        StringMap numberMap = p.getStringMap();

                        for (String key : numberMap.getValMap().keySet()) {
                            String value = numberMap.getValMap().get(key);
                            map.put(key, value);
                        }

                        return (T) map;
                    }
                );
            } 

               

            /*if (genericName.equals("Map<String, Double>")) 
            {
                return new Mutator<T>(
                    (p, v) -> {
                        Map<String, Double> data = (Map<String, Double>) v;
                        DataPackage.Builder builder = dataPackageBuilderCopy(p);

                        NumberMap.Builder numberMapBuilder = NumberMap.newBuilder();
                        for (Map.Entry<String, Double> entry : data.entrySet()) {
                            numberMapBuilder.putVal(entry.getKey(), entry.getValue());
                        }

                        builder.setNumberMap(numberMapBuilder.build());
                        return builder.build();
                    },
                    p -> {
                        Map<String, Double> map = new HashMap<>();
                        NumberMap numberMap = p.getNumberMap();

                        for (String key : numberMap.getValMap().keySet()) {
                            double value = numberMap.getValMap().get(key);
                            map.put(key, value);
                        }

                        return (T) map;
                    }
                );
            } */

        }


         System.out.println("Is protobuf type ? " + dataTypeClass.getName());

        if (GeneratedMessageV3.class.isAssignableFrom(dataTypeClass))
        {
            System.out.println("Is protobuf typ pe in typemapper");
        
            return new Mutator<T>((p, v) -> {
                    GeneratedMessageV3 instance = (GeneratedMessageV3) getProtoMessageInstance((Class<T>) dataTypeClass);
                    final ProtoCodec protoCodec = getProtoCodec(instance);

                    Blob blob = protoCodec.encode((GeneratedMessageV3)v);

                    return dataPackageBuilderCopy(p)
                            .setBlobVal(blob)
                            .build();
            },
            p -> {
                GeneratedMessageV3 instance = (GeneratedMessageV3) getProtoMessageInstance((Class<T>) dataTypeClass);
                final ProtoCodec protoCodec = getProtoCodec(instance);
                
                return (T) protoCodec.decode(p.getBlobVal());
            });
        }
        // Have to use NumberArray, StringArray, ..., since we cannot safely distinguish List<Double> and List<String>?
        // Java generics... best generics... not. Type erasure, great invention.
       /* if (dataType == ArrayList<Double>.class) {
            return new Mutator<T>(
                (p, v) -> dataPackageBuilderCopy(p)
                    .setNumberArrayVal((NumberArray) v)
                    .build(),
                p -> (T) p.getNumberArrayVal()
            );
        }


        if (inst instanceof StringArray) {
            return new Mutator<T>(
                (p, v) -> dataPackageBuilderCopy(p)
                    .setStringArrayVal((StringArray) v)
                    .build(),
                p -> (T) p.getStringArrayVal()
            );
        }

        if (inst instanceof NumberMap) {
            return new Mutator<T>(
                (p, v) -> dataPackageBuilderCopy(p)
                    .setNumberMap((NumberMap) v)
                    .build(),
                p -> (T) p.getNumberMap()
            );
        }

        if (inst instanceof NumberMap) {
            return new Mutator<T>(
                (p, v) -> dataPackageBuilderCopy(p)
                    .setNumberMap((NumberMap) v)
                    .build(),
                p -> (T) p.getNumberMap()
            );
        } */

        
/* 
        // List of Double
        if (inst instanceof List<?>) 
        {
            List<?> list = (List<?>) inst;
            if (!list.isEmpty() && list.get(0) instanceof Double) {
                return new Mutator<>(
                    (p, v) -> p.setNumberArrayVal(new NumberArray((List<Double>) v)),
                    p -> (T) p.getNumberArrayVal().getVal()
                );
            }

            if (!list.isEmpty() && list.get(0) instanceof String) {
                return new Mutator<>(
                    (p, v) -> p.setStringArrayVal(new StringArray((List<String>) v)),
                    p -> (T) p.getStringArrayVal().getVal()
                );
            }
        }

        // Map of Double
        if (inst instanceof Map<?, ?>) {
            Map<?, ?> map = (Map<?, ?>) inst;
            if (!map.isEmpty() && map.values().iterator().next() instanceof Double) {
                return new Mutator<>(
                    (p, v) -> p.setNumberMap(new NumberMap((Map<String, Double>) v)),
                    p -> (T) p.getNumberMap().getVal()
                );
            }

            if (!map.isEmpty() && map.values().iterator().next() instanceof String) {
                return new Mutator<>(
                    (p, v) -> p.setStringMap(new StringMap((Map<String, String>) v)),
                    p -> (T) p.getStringMap().getVal()
                );
            }
        }
*/
        throw new IllegalArgumentException("Type \"" + dataType.getSimpleName() + "\" is not supported by CLAID channels.");
    }
}