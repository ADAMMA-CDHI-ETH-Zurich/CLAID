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

package adamma.c4dhi.claid.RemoteFunction;

import adamma.c4dhi.claid.RemoteFunction.FuturesTable;
import adamma.c4dhi.claid.TypeMapping.DataType;
import adamma.c4dhi.claid.TypeMapping.Mutator;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;
import adamma.c4dhi.claid.Blob;
import adamma.c4dhi.claid.DataPackage;

import java.util.function.Consumer;


public class Future<T> extends AbstractFuture 
{
    private Class<?> returnType = null;
    Consumer<T> callbackConsumer = null;

    public Future(FuturesTable futuresTableInHandler, 
        FutureUniqueIdentifier uniqueIdentifier,
        Class<?> returnType) 
    {
        super(futuresTableInHandler, uniqueIdentifier);
        this.returnType = returnType;
    }

    public T await()
    {
        DataPackage responsePackage = super.awaitResponse();
        if(responsePackage == null || !this.wasExecutedSuccessfully())
        {
            return null;
        }

        return getReturnData(responsePackage);
    }

    private T getReturnData(DataPackage responsePackage)
    {
        if(responsePackage == null)
        {
            return null;
        }
        // For void functions.
        if(this.returnType == null)
        {
            return null;
        }

        if(Void.class.equals(returnType))
        {
            return null;
        }
        
        Mutator<T> mutator = TypeMapping.getMutator(new DataType(this.returnType));

        T t = mutator.getPackagePayload(responsePackage);

        return t;
    }

    public void then(Consumer<T> callbackConsumer)
    {
        this.callbackConsumer = callbackConsumer;
        super.thenUntyped((data) -> callback(data));
    }

    private void callback(DataPackage data)
    {
        if(this.callbackConsumer == null)
        {
            return;
        }
        
        T t = getReturnData(data);
        this.callbackConsumer.accept(t);
    }
}
