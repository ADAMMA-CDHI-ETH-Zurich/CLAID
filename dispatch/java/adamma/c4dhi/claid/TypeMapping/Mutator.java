package adamma.c4dhi.claid.TypeMapping;

import java.util.function.BiFunction;
import java.util.function.Function;

import javax.xml.crypto.Data;

import adamma.c4dhi.claid.DataPackage;

public class Mutator<T> 
{
    // In Java, Protobuf types are immutable. 
    // Hence, the setter has to return a new DataPacke.
    private final BiFunction<DataPackage, T, DataPackage> setter;
    private final Function<DataPackage, T> getter;

    public Mutator(BiFunction<DataPackage, T, DataPackage> setter, Function<DataPackage, T> getter) {
        this.setter = setter;
        this.getter = getter;
    }

    public DataPackage setPackagePayload(DataPackage packet, T value) 
    {
        return setter.apply(packet, value);
    }

    public T getPackagePayload(DataPackage packet) {
        return getter.apply(packet);
    }
}
