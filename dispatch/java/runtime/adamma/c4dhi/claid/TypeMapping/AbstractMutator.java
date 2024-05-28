package adamma.c4dhi.claid.TypeMapping;

import java.util.function.BiFunction;
import java.util.function.Function;


import adamma.c4dhi.claid.DataPackage;

public abstract class AbstractMutator {

    public abstract DataPackage setPackagePayloadFromObject(DataPackage packet, Object o);
    public abstract Object getPackagePayloadAsObject(DataPackage packet);
}
