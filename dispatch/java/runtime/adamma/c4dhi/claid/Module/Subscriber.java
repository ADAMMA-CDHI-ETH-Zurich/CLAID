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

import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.util.function.Consumer;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Scheduling.ConsumerRunnable;
import adamma.c4dhi.claid.Module.Scheduling.RunnableDispatcher;
import adamma.c4dhi.claid.Module.Scheduling.ScheduleOnce;
import adamma.c4dhi.claid.Module.Scheduling.ScheduledRunnable;
import adamma.c4dhi.claid.TypeMapping.DataType;
import adamma.c4dhi.claid.TypeMapping.Mutator;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;
import adamma.c4dhi.claid.DataPackage;


public class Subscriber<T extends Object> extends AbstractSubscriber
{
    private DataType dataType;
    private Consumer<ChannelData<T>> callback;
    private RunnableDispatcher callbackDispatcher;

    private Mutator<T> mutator;
        
    public Subscriber(DataType dataType, Consumer<ChannelData<T>> callback, RunnableDispatcher callbackDispatcher)
    {
        this.dataType = dataType;
        this.callback = callback;
        this.callbackDispatcher = callbackDispatcher;
        this.mutator = TypeMapping.getMutator(dataType);
    }

    private void invokeCallback(ChannelData<T> data)
    {
        ConsumerRunnable<ChannelData<T>> consumerRunnable = new ConsumerRunnable<ChannelData<T>>(data, callback, ScheduleOnce.now());
        ScheduledRunnable runnable = (ScheduledRunnable) consumerRunnable;

        this.callbackDispatcher.addRunnable(runnable);
    }

    @Override
    public void onNewData(DataPackage data) 
    {
        T value = this.mutator.getPackagePayload(data);

        Instant instant = Instant.ofEpochSecond(data.getUnixTimestampMs());

        // Convert Instant to LocalDateTime
        LocalDateTime dateTime = LocalDateTime.ofInstant(instant, ZoneId.systemDefault());

        ChannelData channelData = new ChannelData(value, dateTime, data.getSourceUserToken());

        this.invokeCallback(channelData);

        /*if(dataType.isInstance(data))
        {   
            T typedData = dataType.cast(data);
            invokeCallback(typedData);
        }
        else
        {
            Logger.logError("Error in Java Runtime: Subscriber received data of type \"" + data.getClass().getName() + "\", but\n"
            + "expected data of type \"" + dataType.getName() + "\". Cast failed.");
        }*/
    }

}
