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

import java.util.function.BiFunction;
import java.util.function.Function;


import adamma.c4dhi.claid.DataPackage;

public class Mutator<T> extends AbstractMutator
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

    public T cast(Object o)
    {
        return (T) o;
    }

    @Override
    public DataPackage setPackagePayloadFromObject(DataPackage packet, Object o)
    {
        return setPackagePayload(packet, (T) o);
    }

    @Override
    public Object getPackagePayloadAsObject(DataPackage packet)
    {
        return (Object) getPackagePayload(packet);
    }
}
