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


import java.lang.ProcessBuilder.Redirect.Type;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import adamma.c4dhi.claid.LocalDispatching.ModuleManager;
import adamma.c4dhi.claid.Module.ChannelSubscriberPublisher;
import adamma.c4dhi.claid.Module.ModuleFactory;
import adamma.c4dhi.claid.TypeMapping.DataType;
import adamma.c4dhi.claid.TypeMapping.Mutator;
import adamma.c4dhi.claid.TypeMapping.TypeMapping;
import adamma.c4dhi.claid_platform_impl.CLAID;
import test.TestModule;

public class claid_java_test {
	static void method(ArrayList<Integer> ints, IntegerType ...ignore) {
        System.out.println("Hello, Integer!");
    }
    
    static void method(ArrayList<Double> dbs, DoubleType ...ignore) {
        System.out.println("Hello, Double!");
    }

	static void method(Map<String, Integer> dbs, IntegerType ...ignore) {
        System.out.println("Hello, map integer");
    }

	static void method(Map<String, Double> dbs, DoubleType ...ignore) {
        System.out.println("Hello, map double");
    }

	static void method(Class<?> d) {
        System.out.println("Other");
    }
    
    static class IntegerType {}
    static class DoubleType {}


	public static void main(String[] args){

       

		
		ModuleFactory moduleFactory = new ModuleFactory();
		moduleFactory.registerModule(TestModule.class);

		ArrayList<String> t = new ArrayList<>();
		Mutator<ArrayList<String>> a = TypeMapping.getMutator(new DataType(ArrayList.class, "ArrayList<String>"));



		// If Linux/Android:
		//unix:///tmp/test_socket.grpc
		// Normally unix domain sockets work only on Linux.
		// Currently, however, they do not work at all. To make them work, we need to included netty_shaded instead of netty in our BUILD depndencies.
		// However, for some reason, that currently leads cyclic dependency error. It worked before, but now it does not anymore. Have to look at this again in the future.
		if(!CLAID.start("localhost:1337", "/home/lastchance/Documents/ALEX/alex_config.json", "alex_client", "test", "test", moduleFactory))
		{
			System.exit(0);
		}
		// If macOS or Windows:
		// if(!CLAID.start("localhost:1337", "/Users/planger/Documents/ALEX/alex_config.json", "alex_client", "test", "test", moduleFactory))
		// {
		// 	System.exit(0);
		// }

		System.out.println(TypeMapping.getNewInstance(Double.class).getClass().getName());
		while(true);
	}
}