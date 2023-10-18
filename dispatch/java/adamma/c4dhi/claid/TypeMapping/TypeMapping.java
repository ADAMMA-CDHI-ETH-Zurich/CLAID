package adamma.c4dhi.claid.TypeMapping;

import java.util.List;

import adamma.c4dhi.claid.TypeMapping.Mutator;

import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.TracePoint;

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

    public static <T> Mutator<T> getMutator(T inst) 
    {
        if (inst instanceof Double) {
            return new Mutator<T>(
                (p, v) -> dataPackageBuilderCopy(p)
                    .setNumberVal((Double) v)
                    .build(),
                p -> (T) new Double(p.getNumberVal())
            );
        }

         if (inst instanceof Boolean) {
            return new Mutator<T>(
                (p, v) -> dataPackageBuilderCopy(p)
                    .setBoolVal((Boolean) v)
                    .build(),
                p -> (T) new Boolean(p.getBoolVal())
            );
        }

        /*if (inst instanceof String) {
            return new Mutator<>(
                (p, v) -> p.setStringVal((String) v),
                p -> (T) p.getStringVal()
            );
        }

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
        throw new IllegalArgumentException("Type \"" + inst.getClass().getSimpleName() + "\" is not supported by CLAID channels.");
    }
}