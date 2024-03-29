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

package test.config_reload_test;


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
import test.config_reload_test.TestModule1;

public class config_reload_test {
	

	public static void main(String[] args){

       

		
		ModuleFactory moduleFactory = new ModuleFactory();
		moduleFactory.registerModule(TestModule1.class);

		ArrayList<String> t = new ArrayList<>();
		Mutator<ArrayList<String>> a = TypeMapping.getMutator(new DataType(ArrayList.class, "ArrayList<String>"));


		if(!CLAID.start("localhost:1337", "/Users/planger/Development/ModuleAPIV2/dispatch/test/config_reload_test_1.json", "test_client", "test", "test", moduleFactory))
		{
			System.exit(0);
		}

		System.out.println(TypeMapping.getNewInstance(Double.class).getClass().getName());
		CLAID.loadNewConfig("/Users/planger/Development/ModuleAPIV2/dispatch/test/config_reload_test_2.json");
		while(true);
	}
}