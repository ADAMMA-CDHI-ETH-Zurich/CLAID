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

package test;

import java.time.Duration;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import adamma.c4dhi.claid.Logger.Logger;
import adamma.c4dhi.claid.Module.Channel;
import adamma.c4dhi.claid.Module.ChannelData;
import adamma.c4dhi.claid.Module.Module;


import adamma.c4dhi.claid.DataPackage;
import adamma.c4dhi.claid.Blob;
import adamma.c4dhi.claid.TracePoint;
import adamma.c4dhi.claid.StringArray;
import adamma.c4dhi.claid.NumberArray;
import adamma.c4dhi.claid.NumberMap;
import adamma.c4dhi.claid.StringMap;
import adamma.c4dhi.claid.Module.ModuleAnnotator;

public class TestModule extends Module
{

    private Channel<NumberMap> testChannel;
    private Channel<String> testChannel2;
    private Channel<NumberMap> receiveChannel;
    private Channel<String> receiveChannel2;

    private int ctr = 0;

    public static void annotateModule(ModuleAnnotator annotator)
    {
        annotator.setModuleDescription("Just a simple Module for testing.");
    }

    @Override
    public void initialize(Map<String, String> properties) 
    {
       this.testChannel = this.publish("TestChannel", NumberMap.class);
       this.receiveChannel = this.subscribe("TestChannel", NumberMap.class, val -> onData(val));

    //     this.receiveChannel2 = this.subscribe(String.class, "TestChannel", (data) -> onDataString(data));
    //     this.receiveChannel = this.subscribe(Double.class, "TestChannel", (data) -> onData(data));
    //     this.testChannel2 = this.publish(String.class, "TestChannel");

    //     System.out.println("Hello world from TestModule!");
    //   //  this.testChannel.post(Double.valueOf(42));
        
        
        this.registerPeriodicFunction("TestFunction", () -> testFunction(), Duration.ofMillis(500));
    }

    public void onData(ChannelData<NumberMap> data)
    {
        System.out.println("On data: " + data.getData());
    }

    public void testFunction()
    {
        Logger.logInfo("Test function hello " + ctr);
        Map<String, String> data = new HashMap<>();
        data.put("Test" + ctr, String.valueOf(ctr));

        NumberMap.Builder builder = NumberMap.newBuilder();
        builder.putVal("Test"+ctr, ctr);
        this.testChannel.post(builder.build());
        ctr += 1;

    }

    public void onDataString(String data)
    {

    }

}