package adamma.c4dhi.claid.TypeMapping;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;


import adamma.c4dhi.claid.TypeMapping.Mutator;

import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.TracePoint;
import adamma.c4dhi.claid.StringArray;
import adamma.c4dhi.claid.NumberArray;
import adamma.c4dhi.claid.NumberMap;

import com.google.protobuf.GeneratedMessageV3;

public class TypeMapping {

    public static DataPackage.Builder dataPackageBuilderCopy(DataPackage p)
    {
        DataPackage.Builder builder = DataPackage.newBuilder()
                    .setId(p.getId())
                    .setChannel(p.getChannel())
                    .setSourceHostModule(p.getSourceHostModule())
                    .setTargetHostModule(p.getTargetHostModule())
                    .setSourceUserToken(p.getSourceUserToken())
                    .setTargetUserToken(p.getTargetUserToken())
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
                    GeneratedMessageV3.Builder builder =  (GeneratedMessageV3.Builder) untypedBuilder;
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
                System.out.println("Is protobuf type");
                T instance = getProtoMessageInstance(clz);
                return instance;
            }
            else if (clz == Double.class) {
                System.out.println("Is Double type");
                // For Double, create an instance with a default value of 0.0
                Double instance = 0.0;
                return (T) instance;
            } else if (clz == Integer.class) {
                System.out.println("Is Integer type");
                // For Integer, create an instance with a default value of 0
                Integer instance = 0;
                return (T) instance;
            } 
            else if (clz == Short.class) {
                System.out.println("Is Long type");
                // For Long, create an instance with a default value of 0L
                Short instance = 0;
                return (T) instance;
            } 
            else if (clz == Long.class) {
                System.out.println("Is Long type");
                // For Long, create an instance with a default value of 0L
                Long instance = 0L;
                return (T) instance;
            } else if (clz == Float.class) {
                System.out.println("Is Float type");
                // For Float, create an instance with a default value of 0.0f
                Float instance = 0.0f;
                return (T) instance;
            } else if (clz == Boolean.class) {
                System.out.println("Is Boolean type");
                // For Boolean, create an instance with a default value of false
                Boolean instance = false;
                return (T) instance;
            } 
            else
            {
                System.out.println("Is not protobuf type");
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

    public static <T> Mutator<T> getMutator(Class<T> dataType) 
    {
        if (dataType == Double.class || dataType == Float.class || 
        dataType == Integer.class || dataType == Short.class || dataType == Long.class) {
            return new Mutator<T>(
                (p, v) -> dataPackageBuilderCopy(p)
                    .setNumberVal((Double) v)
                    .build(),
                p -> (T) Double.valueOf(p.getNumberVal())
            );
        }

        if (dataType == Boolean.class) {
            return new Mutator<T>(
                (p, v) -> dataPackageBuilderCopy(p)
                    .setBoolVal((Boolean) v)
                    .build(),
                p -> (T) Boolean.valueOf(p.getBoolVal())
            );
        }

        if (dataType == String.class) {
            return new Mutator<T>(
                (p, v) -> dataPackageBuilderCopy(p)
                    .setStringVal((String) v)
                    .build(),
                p -> (T) p.getStringVal()
            );
        }

        if (dataType == ArrayList.class) {
            // You have a generic ArrayList
            Type genericType = dataType.getGenericSuperclass();
    
            if (genericType instanceof ParameterizedType) {
                ParameterizedType parameterizedType = (ParameterizedType) genericType;
                Type[] typeArguments = parameterizedType.getActualTypeArguments();
    
                if (typeArguments.length == 1 && typeArguments[0] == String.class) {
                    System.out.println("ArrayList of Strings");
                } else if (typeArguments.length == 1 && typeArguments[0] == Double.class) {
                    System.out.println("ArrayList of Doubles");
                }
            }
            return null;
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

        if (GeneratedMessageV3.class.isAssignableFrom(dataType))
        {
            throw new IllegalArgumentException("Protobuf messages not yet supported for Channels.");
        }
        
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