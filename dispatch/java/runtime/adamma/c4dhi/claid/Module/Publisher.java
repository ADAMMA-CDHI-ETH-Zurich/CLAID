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


import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.TypeMapping.DataType;
import adamma.c4dhi.claid.TypeMapping.Mutator;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;

public class Publisher<T> 
{
    private final DataType dataType;
    private final String moduleId;
    private final String channelName;
    private ThreadSafeChannel<DataPackage> toModuleManagerQueue;
    private Mutator<T> mutator;

    public Publisher(DataType dataType, final String moduleId, final String channelName, ThreadSafeChannel<DataPackage> toModuleManagerQueue)
    {
        this.dataType = dataType;
        this.moduleId = moduleId;
        this.channelName = channelName;
        this.toModuleManagerQueue = toModuleManagerQueue;

        this.mutator = TypeMapping.getMutator(dataType);
    }

    public void post(T data, long timestamp)
    {
        DataPackage.Builder builder = DataPackage.newBuilder();
        builder.setSourceModule(this.moduleId);
        builder.setChannel(this.channelName);
        builder.setUnixTimestampMs(timestamp);
        DataPackage dataPackage = builder.build();

        dataPackage = this.mutator.setPackagePayload(dataPackage, data);


        this.toModuleManagerQueue.add(dataPackage);
    }        
}
